
#include "clientheaders.h"
#include "AnimeLineFX.h"
#include "VarTrack.h"
#include "ClientUtilities.h"
#include "RiotClientShell.h"
#include "ltobjectcreate.h"
#include "iltsoundmgr.h"

#define SCALE_UP_TIME	0.3f
#define ROTATE_TIME		0.1f
#define FADE_OUT_TIME	0.4f
#define TOTAL_TIME		(SCALE_UP_TIME+ROTATE_TIME+FADE_OUT_TIME)


static VarTrack g_AnimeNumLines;
static VarTrack g_AnimeScale;
static VarTrack g_AnimeAngleSpeed;
static VarTrack g_AnimeColor;
static VarTrack g_AnimeVariance;



static inline LTFLOAT Rand01()
{
	return (LTFLOAT)rand() / RAND_MAX;
}


CAnimeLineFX::CAnimeLineFX()
{
	m_nLines = 0;
}


CAnimeLineFX::~CAnimeLineFX()
{
	uint32 i;

	if(m_pClientDE)
	{
		for(i=0; i < m_nLines; i++)
		{
			if(m_Lines[i].m_hObject)
			{
				m_pClientDE->RemoveObject(m_Lines[i].m_hObject);
				m_Lines[i].m_hObject = LTNULL;
			}
		}
	}
}


LTBOOL CAnimeLineFX::CreateObject(ILTClient* pClientDE) 
{
	ObjectCreateStruct theStruct;
	uint32 nLines;
	LTFLOAT theColor[3];
	LTFLOAT theScale;
	LTFLOAT angleSpeed;
	LTFLOAT curAngle, angleAdd;
	uint32 i;
	LineInfo *pLine;
	PlaySoundInfo playSoundInfo;


	if (!pClientDE) return LTFALSE;
	m_pClientDE = pClientDE;

	static LTFLOAT theRadius = 1500.0f;
	PLAYSOUNDINFO_INIT(playSoundInfo);
	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_REVERB;
	SAFE_STRCPY(playSoundInfo.m_szSoundName, "sounds\\enemies\\spot\\uca_bad.wav");
	playSoundInfo.m_vPosition = m_BasePos;
	playSoundInfo.m_nPriority	 = SOUNDPRIORITY_PLAYER_HIGH;
	playSoundInfo.m_fOuterRadius = theRadius;
	playSoundInfo.m_fInnerRadius = theRadius * 0.25f;
	playSoundInfo.m_nVolume		 = 100;
	HLTSOUND hSnd = LTNULL;
	m_pClientDE->SoundMgr()->PlaySound(&playSoundInfo, hSnd);



	// Init variables..
	if(!g_AnimeNumLines.IsInitted())
		g_AnimeNumLines.Init(pClientDE, "AnimeNumLines", LTNULL, 6.0f);

	if(!g_AnimeScale.IsInitted())
		g_AnimeScale.Init(pClientDE, "AnimeScale", LTNULL, 45.0f);

	if(!g_AnimeAngleSpeed.IsInitted())
		g_AnimeAngleSpeed.Init(pClientDE, "AnimeAngleSpeed", LTNULL, 15.0f);

	if(!g_AnimeColor.IsInitted())
		g_AnimeColor.Init(pClientDE, "AnimeColor", ".2 .2 .8", 0.0f);

	if(!g_AnimeVariance.IsInitted())
		g_AnimeVariance.Init(pClientDE, "AnimeVariance", LTNULL, 1.2f);

	
	nLines = (uint32)g_AnimeNumLines.GetLTFLOAT();
	theScale = g_AnimeScale.GetLTFLOAT();
	angleSpeed = g_AnimeAngleSpeed.GetLTFLOAT();
	sscanf(g_AnimeColor.GetStr(), "%f %f %f", &theColor[0], &theColor[1], &theColor[2]);



	// Create the lines.
	INIT_OBJECTCREATESTRUCT(theStruct);
	SAFE_STRCPY(theStruct.m_Filename, "models\\multiplaystar.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "skins\\multiplay.dtx");
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT;
	theStruct.m_ObjectType = OT_MODEL;
	theStruct.m_Pos = m_BasePos;

	// Make the line models.
	m_nLines = nLines;
	curAngle = Rand01() * MATH_CIRCLE;
	angleAdd = MATH_CIRCLE / m_nLines;
	for(i=0; i < m_nLines; i++)
	{
		pLine = &m_Lines[i];
		
		pLine->m_hObject = m_pClientDE->CreateObject(&theStruct);
		if(pLine->m_hObject)
		{
			m_pClientDE->SetObjectColor(pLine->m_hObject, theColor[0], theColor[1], theColor[2], 1.0f); 
			
			pLine->m_Angle = curAngle;
			curAngle += angleAdd;

			// Vary it..
			pLine->m_Angle += (Rand01() - 0.5f) * g_AnimeVariance.GetLTFLOAT();

			pLine->m_AngleSpeed = Rand01();
			pLine->m_AngleSpeed = LTMAX(0.3f, pLine->m_AngleSpeed);
			pLine->m_AngleSpeed *= angleSpeed;
			
			pLine->m_TimeToLive = TOTAL_TIME;
			pLine->m_Lifetime = 0.0f;
			pLine->m_Scale = theScale - (Rand01()*theScale*0.4f);
		}
	}

	return LTTRUE;
}


LTBOOL CAnimeLineFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{ 
	if (!psfxCreateStruct) return LTFALSE;

	ALCREATESTRUCT *pAL = (ALCREATESTRUCT*)psfxCreateStruct;
	m_BasePos = pAL->m_Pos;
	m_BaseDir = pAL->m_DirVec;

	return CSpecialFX::Init(psfxCreateStruct);
}


// Return of LTFALSE indicates special fx is done and can be removed.
LTBOOL CAnimeLineFX::Update()
{
	uint32 i, nActive;
	LTVector vScale;
	LTRotation rot;
	LTFLOAT frameTime, r, g, b, a, fMul;
	LineInfo *pLine;


	nActive = 0;
	
	frameTime = m_pClientDE->GetFrameTime();
	for(i=0; i < m_nLines; i++)
	{
		pLine = &m_Lines[i];

		if(pLine->m_hObject)
		{
			pLine->m_Lifetime += frameTime;
			if(pLine->m_Lifetime > pLine->m_TimeToLive)
			{
				m_pClientDE->RemoveObject(pLine->m_hObject);
				pLine->m_hObject = LTNULL;
				continue;
			}

			VEC_SET(vScale, pLine->m_Scale, pLine->m_Scale*0.5f, pLine->m_Scale*0.5f);
			if(pLine->m_Lifetime < SCALE_UP_TIME)
			{
				fMul = pLine->m_Lifetime / SCALE_UP_TIME;
				vScale *= fMul;
				m_pClientDE->SetObjectScale(pLine->m_hObject, &vScale);
			}
			else
			{
				m_pClientDE->SetObjectScale(pLine->m_hObject, &vScale);
			}

			// Update its rotation.
			if(pLine->m_Lifetime > SCALE_UP_TIME)
			{
				pLine->m_Angle += pLine->m_AngleSpeed * frameTime;
			}
			
			m_pClientDE->Math()->AlignRotation(rot, m_BaseDir, LTVector(0, 1, 0));
			m_pClientDE->Math()->RotateAroundAxis(rot, m_BaseDir, pLine->m_Angle);
			m_pClientDE->SetObjectRotation(pLine->m_hObject, &rot);
			
			// Set alpha.
			if(pLine->m_Lifetime > (SCALE_UP_TIME+ROTATE_TIME))
			{
				fMul = (pLine->m_Lifetime - (SCALE_UP_TIME+ROTATE_TIME)) / FADE_OUT_TIME;
				m_pClientDE->GetObjectColor(pLine->m_hObject, &r, &g, &b, &a);
				a = 1.0f - fMul;
				m_pClientDE->SetObjectColor(pLine->m_hObject, r, g, b, a);
			}

			++nActive;
		}
	}
	
	return nActive > 0;
}



