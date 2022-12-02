#include "stdafx.h"
#include "..\Public\Player.h"
#include "GameInstance.h"
#include "ImGuiMgr.h"
#include "HierarchyNode.h"

#include "Status.h"

#include "MotionTrail.h"
#include "Dummy.h"
#include "Weapon.h"
#include "Bow.h"

#include "Layer.h"

#include "Monster.h"

#include "CameraMgr.h"
#include "Camera_Player.h"
#include "Camera_CutScene_Enter.h"
#include "Camera_CutScene.h"

#include "Navigation.h"

#include "UI_Targeting.h"
#include "UI_Mgr.h"
#include "SkillSlot.h"

#include "Extra01.h"
#include "Extra02.h"
#include "Extra01_Last.h"
#include "Extra02_Last.h"
#include "Magician.h"
#include "Puppet.h"

#include "Timer_Manager.h"

#include "Effect_Particle.h"
#include "Effect_Mgr.h"
#include "CrossTrail.h"

#include "Bow.h"
#include "Saber.h"

_uint			CPlayer::m_iStage = 0;

CPlayer::CPlayer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject(pDevice, pContext)
{
	for (_uint i = 0; i < 2; ++i)
	{
		m_pBones[i] = nullptr;
		m_pHands[i] = nullptr;
	}
	
}

CPlayer::CPlayer(const CPlayer & rhs)
	: CGameObject(rhs)
	, m_ePass(rhs.m_ePass)
	, m_AnimPos(rhs.m_AnimPos)
	, m_PreAnimPos(rhs.m_PreAnimPos)
{
	for (_uint i = 0; i < 2; ++i)
	{
		m_pBones[i] = nullptr;
		m_pHands[i] = nullptr;
	}
}

HRESULT CPlayer::Initialize_Prototype()
{
	XMStoreFloat4(&m_AnimPos, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	m_PreAnimPos = m_AnimPos;


	return S_OK;
}

HRESULT CPlayer::Initialize(void * pArg)
{

	AUTOINSTANCE(CGameInstance, pGameInstance);
	AUTOINSTANCE(CCameraMgr, pCameraMgr);

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_Hands()))
		return E_FAIL;

	if (FAILED(Ready_PlayerParts()))
		return E_FAIL;

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	Ready_AnimLimit();
	Ready_ParticleDesc();

	ZeroMemory(&m_tInfo, sizeof(OBJ_DESC));

	if (pArg)
	{
		OBJ_DESC _tInfo = *static_cast<OBJ_DESC*>(pArg);
		Set_Info(_tInfo);
	}


	pGameInstance->Set_Player(this);



	m_fAnimSpeed = 1.f;

	for (int i = 0; i < 10; ++i)
	{
		//m_listMotion
		CMotionTrail* _Motion = CMotionTrail::Create(m_pDevice, m_pContext, m_pModelCom);
		if (_Motion == nullptr)
		{
			MSG_BOX(TEXT("creat Motion"));
			return E_FAIL;
		}
		CDummy*		_pDummy = CDummy::Creat(m_pDevice, m_pContext, _Motion, _float4x4());
		m_listDeadMotion.push_back(_pDummy);
	}

	KeySetting();


	m_MonsterLayer = pGameInstance->Get_Layer(g_eCurLevel, TEXT("Layer_Monster"));
	switch (g_eCurLevel)
	{
	case Client::LEVEL_GAMEPLAY:
		m_pTransformCom->Turn_Angle(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(-90.f));
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(82.971f, 0.882f, 40.563f, 1.f));
		break;
	case Client::LEVEL_STAGE_02_1:
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(51.979f, 0.115f, -7.650f, 1.f));
		break;
	case Client::LEVEL_STAGE_02:
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(44.083f, 10.f, 17.580f, 1.f));
		//pCameraMgr->Change_Camera(CCameraMgr::CAMERA_CUTSCENE_ENTER);
		m_eCurState = Corvus_SD_Fall_Loop;
		break;
	case Client::LEVEL_STAGE_LAST:
		m_pNavigationCom->Set_Index(388);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(41.614f, 29.997f, 79.284f, 1.f));
		break;
	case Client::LEVEL_STAGE_LOBBY:
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(34.491f, 0.165f, 44.676f, 1.f));
		break;
	}
	m_eTypeObj = CGameObject::TYPE_PLAYER;

	POINTLIGHTDESC LIGHT;
	LIGHT.fRange = 1.5f;
	LIGHT.vDiffuse = CLIENT_RGB(119.f, 245.f, 200.f);
	LIGHT.vSpecular = _float4(0.01f, 0.03f, 0.01f, 1.f);
	LIGHT.vAmbient = _float4(0.1f, 0.3f, 0.1f, 1.f);
	LIGHT.vPosition = _float4(0.f, 0.f, 0.f, 1.f);
	LIGHT.LightViewMatrix = new _float4x4;
	XMStoreFloat4x4(LIGHT.LightViewMatrix, XMMatrixIdentity());
	LIGHT.LightProjMatrix = new _float4x4;
	XMStoreFloat4x4(LIGHT.LightProjMatrix, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(60.f), 1280.f / 720.f, 0.2f, 300.f)));
	m_iClawLight = pGameInstance->Add_Light(m_pDevice, m_pContext, g_eCurLevel, CLight_Manager::DYNAMICPOINTLIHGT, LIGHT, -0.3f, 0.1f);


	return S_OK;
}

void CPlayer::Tick(_float fTimeDelta)
{
	ImGuiTick();
	if (m_pModelCom != nullptr)
	{
		if (!m_bAnimStop)
		{
			KeyInputMain(fTimeDelta);
			CheckAnim();
			//CheckState();
			AfterAnim(fTimeDelta);
			PlayAnimation(fTimeDelta);

		}
		else
		{
			CheckAnim();
			_float4 _vAnim;
			XMStoreFloat4(&_vAnim, XMVectorSet(0.f, 0.f, 0.f, 1.f));
			if (m_pModelCom->Play_Animation(fTimeDelta, &_vAnim, &m_fPlayTime, m_bAgainAnim))
			{
				//CheckEndAnim(fTimeDelta);
			}
			CheckLimit();
			XMStoreFloat4(&m_AnimPos, (XMLoadFloat4(&_vAnim) - XMLoadFloat4(&m_PreAnimPos)));
			m_PreAnimPos = _vAnim;
		}


	}

	if (m_pTarget)
	{
		TargetCheck();
	}

	Update_Weapon(fTimeDelta);

	Update_Collider();

	Check_MotionTrail(fTimeDelta);

	AUTOINSTANCE(CGameInstance, _pGame);
	_float4	_vPos;
	XMStoreFloat4(&_vPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
	_pGame->Set_PlayerPos(_vPos);
}

void CPlayer::LateTick(_float fTimeDelta)
{
	if (Collision(fTimeDelta))
	{
		CheckAnim();
		AfterAnim(fTimeDelta);
		PlayAnimation(fTimeDelta);
	}

	if (m_pTarget)
	{
		CUI_Targeting* _pUI_Targeting = static_cast<CUI_Targeting*>(CUI_Mgr::Get_Instance()->Get_UI(TEXT("Targeting")));
		if (_pUI_Targeting)
		{
			_pUI_Targeting->Targeting(
				m_pTransformCom->Get_State(CTransform::STATE_POSITION),
				static_cast<CTransform*>(m_pTarget->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION)
			);
		}
	}

	Add_Render();
}

HRESULT CPlayer::Render()
{
	if (nullptr == m_pModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;
	AUTOINSTANCE(CGameInstance, _pInstance);
	_uint		iNumMeshes;//메쉬 갯수를 알고 메쉬 갯수만큼 렌더를 할 것임. 여기서!

	SetUp_ShaderResources();
	
	iNumMeshes = m_pModelCom->Get_NumMesh();//메쉬 갯수를 알고 메쉬 갯수만큼 렌더를 할 것임. 여기서!

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;

		if(FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_NORMALS, "g_NormalTexture")))
			return E_FAIL;
		
		_float4 _vCamPosition = _pInstance->Get_CamPosition();

		m_pShaderCom->Set_RawValue("g_vCamPosition", &_vCamPosition, sizeof(_float4));

		//DIRLIGHTDESC* _DirLightDesc = _pInstance->Get_DirLightDesc(g_eCurLevel, 0);
		//if (_DirLightDesc != nullptr)
		//{
		//	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", (_DirLightDesc->LightDirInverseMatrix), sizeof(_float4x4))))
		//		return E_FAIL;
		//	if (FAILED(m_pModelCom->Render(m_pShaderCom, 8, i)))
		//		return E_FAIL;
		//}
		

		

		if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &_pInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
			return E_FAIL;
		if (FAILED(m_pModelCom->Render(m_pShaderCom, m_ePass, i)))
			return E_FAIL;


		/*if (FAILED(m_pModelCom->Render(m_pShaderCom, 5, i)))
			return E_FAIL;*/
	}

#ifdef _DEBUG
	/*for (_uint i = 0; i < COLLILDERTYPE_END; ++i)
	{
	if (nullptr != m_pColliderCom[i])
	m_pColliderCom[i]->Render();
	}*/

	//m_pNavigationCom->Render();

#endif


	return S_OK;
}

HRESULT CPlayer::Render_Shadow()
{
	if (nullptr == m_pModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;
	AUTOINSTANCE(CGameInstance, _pInstance);
	_uint		iNumMeshes;//메쉬 갯수를 알고 메쉬 갯수만큼 렌더를 할 것임. 여기서!

	SetUp_ShaderResources();

	iNumMeshes = m_pModelCom->Get_NumMesh();//메쉬 갯수를 알고 메쉬 갯수만큼 렌더를 할 것임. 여기서!

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;

		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_NORMALS, "g_NormalTexture")))
			return E_FAIL;

		_float4 _vCamPosition = _pInstance->Get_CamPosition();

		m_pShaderCom->Set_RawValue("g_vCamPosition", &_vCamPosition, sizeof(_float4));

		DIRLIGHTDESC* _DirLightDesc = _pInstance->Get_DirLightDesc(g_eCurLevel, 0);

		
		if (_pInstance->Get_LightMatrix(g_eCurLevel, CLight_Manager::LIGHT_FIRST, CLight_Manager::LIGHT_VIEW) != nullptr)
		{
			if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", _pInstance->Get_LightMatrix(g_eCurLevel, CLight_Manager::LIGHT_FIRST, CLight_Manager::LIGHT_VIEW), sizeof(_float4x4))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", _pInstance->Get_LightMatrix(g_eCurLevel, CLight_Manager::LIGHT_FIRST, CLight_Manager::LIGHT_PROJ), sizeof(_float4x4))))
				return E_FAIL;
			if (FAILED(m_pModelCom->Render(m_pShaderCom, 8, i)))
				return E_FAIL;

			if (_pInstance->Get_LightMatrix(g_eCurLevel, CLight_Manager::LIGHT_SECOND, CLight_Manager::LIGHT_VIEW) != nullptr)
			{
				if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix2", _pInstance->Get_LightMatrix(g_eCurLevel, CLight_Manager::LIGHT_SECOND, CLight_Manager::LIGHT_VIEW), sizeof(_float4x4))))
					return E_FAIL;
				if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix2", _pInstance->Get_LightMatrix(g_eCurLevel, CLight_Manager::LIGHT_SECOND, CLight_Manager::LIGHT_PROJ), sizeof(_float4x4))))
					return E_FAIL;
				if (FAILED(m_pModelCom->Render(m_pShaderCom, 9, i)))
					return E_FAIL;
			}

			
		}
	}
	return S_OK;
}

void CPlayer::PlayAnimation(_float fTimeDelta)
{
	if (m_bAnimStop)
		return;
	_float4 _vAnim;
	XMStoreFloat4(&_vAnim, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	if (m_pModelCom->Play_Animation(fTimeDelta, &_vAnim, &m_fPlayTime, m_bAgainAnim))
	{
		CheckEndAnim();
	}
	CheckLimit();
	XMStoreFloat4(&m_AnimPos, (XMLoadFloat4(&_vAnim) - XMLoadFloat4(&m_PreAnimPos)));
	m_PreAnimPos = _vAnim;
}

void CPlayer::Set_Info(OBJ_DESC _tInfo)
{
	m_tInfo.eLevel = _tInfo.eLevel;

	if (m_pModelCom == nullptr)
	{
		m_tInfo.szModelTag = _tInfo.szModelTag;
		__super::Add_Component(LEVEL_GAMEPLAY, m_tInfo.szModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);

	}
	else if (lstrcmp(m_tInfo.szModelTag, _tInfo.szModelTag))
	{
		Safe_Delete_Array(m_tInfo.szModelTag);
		Safe_Release(m_pModelCom);

		auto& iter = find_if(m_Components.begin(), m_Components.end(), CTag_Finder(TEXT("Com_Model")));
		Safe_Release(iter->second);
		m_Components.erase(iter);

		m_tInfo.szModelTag = _tInfo.szModelTag;
		__super::Add_Component(LEVEL_GAMEPLAY, m_tInfo.szModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom);
	}
	m_pModelCom->Set_AnimationIndex(STATE_AVOIDATTACK);
	m_tInfo.matWorld = _tInfo.matWorld;
	m_pTransformCom->Set_WorldFloat4x4(m_tInfo.matWorld);
}

void CPlayer::ImGuiTick()
{
}

void CPlayer::KeySetting()
{
	//KeyInput.resize(STATE_END);
	KeyInput[STATE_ATT1] = &CPlayer::KP_ATT;
	KeyInput[STATE_ATT2] = &CPlayer::KP_ATT;
	KeyInput[STATE_ATT3] = &CPlayer::KP_ATT;
	KeyInput[STATE_ATT4] = &CPlayer::KP_ATT;
	KeyInput[STATE_ATT5] = &CPlayer::KP_ATT;
	KeyInput[STATE_IDLE] = &CPlayer::KeyInput_Idle;
	KeyInput[STATE_RUN_F] = &CPlayer::KeyInput_Idle;
}

void CPlayer::KeyInputMain(_float fTimeDelta)
{
	/*for (int i = 0; i< KeyInput[m_eCurState].size(); ++i)
	(this->*(KeyInput[m_eCurState][i]))(fTimeDelta);*/
	//KeyInput[m_eCurState](fTimeDelta);

	switch (m_eCurState)
	{
	case Client::CPlayer::STATE_ATT1:
		KP_ATT(fTimeDelta);
		break;
	case Client::CPlayer::STATE_ATT2:
		KP_ATT(fTimeDelta);
		break;
	case Client::CPlayer::STATE_ATT3:
		KP_ATT(fTimeDelta);
		break;
	case Client::CPlayer::STATE_ATT4:
		KP_ATT(fTimeDelta);
		break;
	case Client::CPlayer::STATE_ATT5:
		break;
	case Client::CPlayer::STATE_RUN_B:
		KeyInput_Idle(fTimeDelta);
		break;
	case Client::CPlayer::STATE_RUN_F:
		KeyInput_Idle(fTimeDelta);
		break;
	case Client::CPlayer::STATE_RUN_L:
		KeyInput_Idle(fTimeDelta);
		break;
	case Client::CPlayer::STATE_RUN_R:
		KeyInput_Idle(fTimeDelta);
		break;
	case Client::CPlayer::STATE_APPROACH:
		
		break;
	case Client::CPlayer::STATE_IDLE:
		KeyInput_Idle(fTimeDelta);
		break;
	case Client::CPlayer::STATE_WALK:
		KeyInput_Idle(fTimeDelta);
		break;
	case Client::CPlayer::STATE_AVOIDATTACK:
		KP_AVOIDATTACK(fTimeDelta);
		break;
	case Client::CPlayer::STATE_JUMPAVOID:

		break;
	case Client::CPlayer::STATE_AVOIDBACK:
		KP_AVOIDATTACK(fTimeDelta);
		break;
	case Client::CPlayer::Corvus_PW_Axe:
		break;
	case Client::CPlayer::Tackle_F:
		break;
	case Client::CPlayer::ParryR:
		KP_Parry(fTimeDelta);
		break;
	case Client::CPlayer::ParryL:
		KP_Parry(fTimeDelta);
		break;
	case Client::CPlayer::DualKnife:
		break;
	case Client::CPlayer::GreatSword:
		break;
	case Client::CPlayer::PW_Halberds:
		break;
	case Client::CPlayer::PW_Hammer_A:
		break;
	case Client::CPlayer::PW_TwinSwords_1:
		break;
	case Client::CPlayer::PW_TwinSwords_2:
		break;
	case Client::CPlayer::PW_VargSword_A:
		break;
	case Client::CPlayer::PW_Bow_Start:
		break;
	case Client::CPlayer::PW_Bow_End:
		break;
	case Client::CPlayer::PW_Bow_B:
		break;
	case Client::CPlayer::PW_Bloodwhip:
		break;
	case Client::CPlayer::PW_CaneSword_SP01:
		break;
	case Client::CPlayer::PW_CaneSword_SP02:
		break;
	case Client::CPlayer::Healing_Little:
		break;
	case Client::CPlayer::Healing2_Blend:
		break;
	case Client::CPlayer::Raven_ClawCommonV2_ChargeStart:
		break;
	case Client::CPlayer::Raven_ClawLong_ChargeFull:
		break;
	case Client::CPlayer::Raven_ClawNear:
		KP_ClawNear(fTimeDelta);
		break;
	case Client::CPlayer::Strong_Jump:
		break;
	case Client::CPlayer::RavenAttack_Start:
		break;
	case Client::CPlayer::RavenAttack_End:
		break;
	case Client::CPlayer::SD_ParryToMob:
		break;
	case Client::CPlayer::SD_HurtIdle:
		KP_AVOIDATTACK(fTimeDelta);
		break;
	case Client::CPlayer::SD_StrongHurt_Start:
		break;
	case Client::CPlayer::SD_StrongHurt_End:
		break;
	case Client::CPlayer::SD_Dead:
		break;
	case Client::CPlayer::STATE_RUN_BL:
		KeyInput_Idle(fTimeDelta);
		break;
	case Client::CPlayer::STATE_RUN_BR:
		KeyInput_Idle(fTimeDelta);
		break;
	case Client::CPlayer::STATE_RUN_FL:
		KeyInput_Idle(fTimeDelta);
		break;
	case Client::CPlayer::STATE_RUN_FR:
		KeyInput_Idle(fTimeDelta);
		break;
	case Client::CPlayer::STATE_END:
		break;
	default:
		//KeyInput_Idle(fTimeDelta);
		break;
	}
}

void CPlayer::KeyInput_Idle(_float fTimeDelta)
{
	AUTOINSTANCE(CGameInstance, pGameInstance);
	if (m_pTarget)
	{
		TargetingMove(fTimeDelta);
	}
	else
	{
		Move(fTimeDelta);
	}

	if (pGameInstance->KeyDown(DIK_P))
	{
		//CEffect_Particle::OPTION _tOption;
		//_tOption.Center = _float3(45.f, 2.f, 45.f);
		//_vector	_vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		//_float4x4	_vWorld = m_pTransformCom->Get_WorldFloat4x4();;
		////_tOption.Center = _float3(_vPos.m128_f32[0], _vPos.m128_f32[1] + 1.f, _vPos.m128_f32[2]);
		//_tOption.eType = CEffect_Particle::PARTICLETYPE::TYPE_STRIGHT;
		//_tOption.fAccSpeed = 0.99f;
		//_tOption.fSpeed = { 2.5f, 3.3f };
		//_tOption.fGravity = 0.f;
		//_tOption.fLifeTime = 0.f;
		//_tOption.fRange = _float3(5.f, 5.f, 1.f);
		//_tOption.iNumParticles = 15;
		//_tOption.Size = _float2(0.3f, 0.3f);
		//_tOption.Spread = CEffect_Particle::SPREAD::SPREAD_EDGE;
		//_tOption.szMaskTag = TEXT("Prototype_Component_Texture_Mask_Blood");
		//_tOption.szTextureTag = TEXT("Prototype_Component_Texture_Diffuse_Blood");
		////_tOption.vColor = CLIENT_RGB(119.f, 245.f, 200.f);
		////_tOption.vColor = CLIENT_RGB(82.f, 9.f, 4.f);
		////_tOption.bPlayerDir = true;
		//_tOption.fSpead_Angle = _float3(0.f, 20.f, 20.f);
		//_tOption.vStart_Dir = _float3(1.f, 0.f, 0.f);
		//_tOption.eDiffuseType = CEffect_Particle::DIFFUSE_TEXTURE;
		//_tOption.eDirType = CEffect_Particle::DIR_TYPE::DIR_ANGLE;
		//_tOption.eStartType = CEffect_Particle::START_CENTER;
		//_tOption.fMaxDistance = { 0.4f, 1.2f };
		//_tOption.bPlayerDir = false;
		//_tOption.matPlayerAxix = _vWorld;

		//if (FAILED(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_PARTICLE, &_tOption)))
		//{
		//	MSG_BOX(TEXT("effect"));
		//	return;
		//}

		CEffect_Particle::OPTION _tOption;
		//_tOption.Center = _float3(45.f, 2.f, 45.f);
		_vector	_vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		_float4x4	_vWorld = m_pTransformCom->Get_WorldFloat4x4();;
		_tOption.Center = _float3(_vPos.m128_f32[0], _vPos.m128_f32[1] + 1.f, _vPos.m128_f32[2]);
		_tOption.eType = CEffect_Particle::PARTICLETYPE::TYPE_STRIGHT;
		_tOption.fAccSpeed = 0.99f;
		_tOption.fSpeed = { 1.5f, 3.3f };
		_tOption.fGravity = 0.f;
		_tOption.fLifeTime = 0.f;
		_tOption.fRange = _float3(5.f, 5.f, 1.f);
		_tOption.iNumParticles = 30;
		_tOption.Size = _float2(0.2f, 0.2f);
		_tOption.Spread = CEffect_Particle::SPREAD::SPREAD_EDGE;
		_tOption.szMaskTag = TEXT("Prototype_Component_Texture_Mask_Blood");
		_tOption.szTextureTag = TEXT("Prototype_Component_Texture_Diffuse_Blood");
		_tOption.vColor = CLIENT_RGB(119.f, 245.f, 200.f);
		//_tOption.vColor = CLIENT_RGB(82.f, 9.f, 4.f);
		//_tOption.bPlayerDir = true;
		_tOption.fSpead_Angle = _float3(0.f, 10.f, 10.f);
		_tOption.vStart_Dir = _float3(1.f, 0.f, 0.f);
		_tOption.eDiffuseType = CEffect_Particle::DIFFUSE_COLOR;
		_tOption.eDirType = CEffect_Particle::DIR_TYPE::DIR_ANGLE;
		_tOption.eStartType = CEffect_Particle::START_CENTER;
		_tOption.fMaxDistance = { 0.4f, 1.2f };
		_tOption.bPlayerDir = true;
		_tOption.matPlayerAxix = _vWorld;

		if (nullptr == CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_PARTICLE, &_tOption))
		{
			MSG_BOX(TEXT("fail effect"));
			return;
		}

	}
	
	//if (CGameInstance::Get_Instance()->KeyDown(DIK_SPACE))
	if (pGameInstance->MouseDown(DIMK_LBUTTON))
	{
		m_eCurState = STATE_ATT1;
	}

	if (pGameInstance->KeyDown(DIK_F))
	{
		m_eCurState = ParryL;
	}

	if (pGameInstance->KeyDown(DIK_C))
	{
		if (m_pTarget)
		{
			m_pTransformCom->LookAt_ForLandObject(static_cast<CTransform*>(m_pTarget->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION));
			m_eCurState = Corvus_Raven_ClawLong_ChargeFull;
			m_eWeapon = WEAPON_NONE;
		}
	}

	if (pGameInstance->KeyDown(DIK_SPACE))
	{
		m_eCurState = STATE_AVOIDATTACK;
		if (pGameInstance->KeyPressing(DIK_S))
		{
			m_eCurState = STATE_AVOIDBACK;
			m_eWeapon = WEAPON_BASE;
		}
	}

	if (pGameInstance->MouseDown(DIMK_RBUTTON))
	{
		m_eCurState = Raven_ClawNear;
	}

	if (pGameInstance->KeyDown(DIK_R))
	{
		m_eCurState = Healing_Little;
	}
	if (pGameInstance->KeyDown(DIK_1))
	{
		if (static_cast<CSkillSlot*>(CUI_Mgr::Get_Instance()->Get_UI(TEXT("PLAYER_SLOT")))->Check_CoolDown(CSkillSlot::SKILL_1))
		{
			m_eCurState = DualKnife;
			m_eWeapon = WEAPON::WEAPON_SKILL;
			m_eCurSkill = SKILL_DUAL;
			m_bCollision[COLLIDERTYPE_BODY] = false;
		}
		
	}
	if (pGameInstance->KeyDown(DIK_2))
	{
		if (static_cast<CSkillSlot*>(CUI_Mgr::Get_Instance()->Get_UI(TEXT("PLAYER_SLOT")))->Check_CoolDown(CSkillSlot::SKILL_2))
		{
			m_eCurState = Corvus_PW_Axe;
			m_bCollision[COLLIDERTYPE_BODY] = false;
		}
		
	}
	if (pGameInstance->KeyDown(DIK_3))
	{
		if (static_cast<CSkillSlot*>(CUI_Mgr::Get_Instance()->Get_UI(TEXT("PLAYER_SLOT")))->Check_CoolDown(CSkillSlot::SKILL_3))
		{
			m_eCurState = PW_Bow_Start;
			m_eWeapon = WEAPON::WEAPON_SKILL;
			m_eCurSkill = SKILL_BOW;
			m_bCollision[COLLIDERTYPE_BODY] = false;
			static_cast<CBow*>(m_pSkillParts[SKILL_BOW][0])->Set_Stop(false);
			m_pModelCom->DirectAnim(PW_Bow_Start);
			static_cast<CAnimModel*>(m_pSkillParts[SKILL_BOW][0]->Get_ComponentPtr(TEXT("Com_Model")))->Set_AnimationIndex(CBow::BOW_START);
			static_cast<CAnimModel*>(m_pSkillParts[SKILL_BOW][0]->Get_ComponentPtr(TEXT("Com_Model")))->DirectAnim(CBow::BOW_START);
		}
		
	}
	if (pGameInstance->KeyDown(DIK_4))
	{
		if (static_cast<CSkillSlot*>(CUI_Mgr::Get_Instance()->Get_UI(TEXT("PLAYER_SLOT")))->Check_CoolDown(CSkillSlot::SKILL_4))
		{
			m_eWeapon = WEAPON::WEAPON_SKILL;
			m_eCurSkill = SKILL_SCYTHE;

			m_eCurState = Corvus_PW_Scythe;
			m_bCollision[COLLIDERTYPE_BODY] = false;
			m_pSkillParts[SKILL_SCYTHE][0]->Set_CollisionOn(true);
			m_pStatusCom->Set_Attack(m_fSkillAtts[SKILL_SCYTHE] * m_fUseStealSkill);
			_bool _bFire(true);
			m_pSkillParts[SKILL_SCYTHE][0]->Clear(&_bFire);
		}
	}
	if (pGameInstance->KeyDown(DIK_5))
	{
		if (static_cast<CSkillSlot*>(CUI_Mgr::Get_Instance()->Get_UI(TEXT("PLAYER_SLOT")))->Check_CoolDown(CSkillSlot::SKILL_5))
		{
			m_eWeapon = WEAPON::WEAPON_SKILL;
			m_eCurSkill = SKILL_LENCE;

			m_eCurState = Corvus_PW_Halberds;
			m_bCollision[COLLIDERTYPE_BODY] = false;
		}
	}

	if (pGameInstance->KeyDown(DIK_T))
	{
		if (m_eStealSkill != STATE_END)
		{
			static_cast<CSkillSlot*>(CUI_Mgr::Get_Instance()->Get_UI(TEXT("PLAYER_SLOT")))->Set_Skill(-1);
			switch (m_eStealSkill)
			{
			case Client::CPlayer::Corvus_PW_Axe:
				m_eCurState = Corvus_PW_Axe;
				m_bCollision[COLLIDERTYPE_BODY] = false;
				break;
			case Client::CPlayer::DualKnife:
				m_eCurState = DualKnife;
				m_eWeapon = WEAPON::WEAPON_SKILL;
				m_eCurSkill = SKILL_DUAL;
				m_bCollision[COLLIDERTYPE_BODY] = false;
				break;
			case Client::CPlayer::PW_Bow_Start:
				m_eCurState = PW_Bow_Start;
				m_eWeapon = WEAPON::WEAPON_SKILL;
				m_eCurSkill = SKILL_BOW;
				m_bCollision[COLLIDERTYPE_BODY] = false;
				static_cast<CBow*>(m_pSkillParts[SKILL_BOW][0])->Set_Stop(false);
				m_pModelCom->DirectAnim(PW_Bow_Start);
				static_cast<CAnimModel*>(m_pSkillParts[SKILL_BOW][0]->Get_ComponentPtr(TEXT("Com_Model")))->Set_AnimationIndex(CBow::BOW_START);
				static_cast<CAnimModel*>(m_pSkillParts[SKILL_BOW][0]->Get_ComponentPtr(TEXT("Com_Model")))->DirectAnim(CBow::BOW_START);
				break;
			case Client::CPlayer::Corvus_PW_Halberds:
				m_eWeapon = WEAPON::WEAPON_SKILL;
				m_eCurSkill = SKILL_LENCE;

				m_eCurState = Corvus_PW_Halberds;
				m_bCollision[COLLIDERTYPE_BODY] = false;
				break;
			case Client::CPlayer::Corvus_PW_Scythe:
				m_eWeapon = WEAPON::WEAPON_SKILL;
				m_eCurSkill = SKILL_SCYTHE;

				m_eCurState = Corvus_PW_Scythe;
				m_bCollision[COLLIDERTYPE_BODY] = false;
				m_pSkillParts[SKILL_SCYTHE][0]->Set_CollisionOn(true);
				m_pStatusCom->Set_Attack(m_fSkillAtts[SKILL_SCYTHE] * m_fUseStealSkill);
				_bool _bFire(true);
				m_pSkillParts[SKILL_SCYTHE][0]->Clear(&_bFire);
				break;
			}
			m_eStealSkill = STATE_END;
			m_fUseStealSkill = 2.f;
		}
	}

	if (pGameInstance->KeyDown(DIK_6))
	{
		static_cast<CSkillSlot*>(CUI_Mgr::Get_Instance()->Get_UI(TEXT("PLAYER_SLOT")))->Reset();
	}

	if (pGameInstance->MouseDown(DIMK_WHEEL))
	{
		Targeting();
	}
	if (g_eCurLevel == LEVEL_STAGE_02_1 || g_eCurLevel == LEVEL_STAGE_LAST)
	{
		if (pGameInstance->KeyDown(DIK_Q))
		{
			
			Execution();
		}
	}
}

void CPlayer::Move(_float fTimeDelta)
{
	AUTOINSTANCE(CGameInstance, pGameInstance);
	if (pGameInstance->KeyPressing(DIK_A))
	{
		if (pGameInstance->KeyPressing(DIK_W))
		{
			m_eDir = DIR_FL;
		}
		else if (pGameInstance->KeyPressing(DIK_S))
		{
			m_eDir = DIR_BL;
		}
		else
		{
			m_eDir = DIR_L;
		}
	}
	else if (pGameInstance->KeyPressing(DIK_D))
	{
		if (pGameInstance->KeyPressing(DIK_W))
		{
			m_eDir = DIR_FR;
		}
		else if (pGameInstance->KeyPressing(DIK_S))
		{
			m_eDir = DIR_BR;
		}
		else
		{
			m_eDir = DIR_R;
		}
	}
	else
	{
		if (pGameInstance->KeyPressing(DIK_W))
		{
			m_eDir = DIR_F;
		}
		else if (pGameInstance->KeyPressing(DIK_S))
		{
			m_eDir = DIR_B;
		}
		else
		{
			m_eDir = DIR_END;
		}
	}

	_float4x4 _CamMatix;
	XMStoreFloat4x4(&_CamMatix, XMMatrixInverse(nullptr, pGameInstance->Get_TransformMatrix(CPipeLine::D3DTS_VIEW)));

	_vector	_vCamLook, _vCamRight, _vLook, _vRight;
	_float _fRatio = 0.8f;
	_vCamLook = XMLoadFloat4((_float4*)&_CamMatix.m[2][0]);
	_vCamRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), _vCamLook));
	_vCamLook = XMVector3Normalize(XMVector3Cross(_vCamRight, XMVectorSet(0.f, 1.f, 0.f, 0.f)));

	_vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	_vRight = m_pTransformCom->Get_State(CTransform::STATE_RIGHT);

	switch (m_eDir)
	{
	case Client::CPlayer::DIR_F:
		m_eCurState = STATE_RUN_F;
		m_pTransformCom->Turn(_vLook, _vCamLook, _fRatio);
		m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_B:
		m_eCurState = STATE_RUN_F;

		m_pTransformCom->Turn(_vLook, _vCamLook * (-1.f), _fRatio);
		m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_R:
		m_eCurState = STATE_RUN_F;

		m_pTransformCom->Turn(_vLook, _vCamRight, _fRatio);
		m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_L:
		m_eCurState = STATE_RUN_F;

		m_pTransformCom->Turn(_vLook, _vCamRight * (-1.f), _fRatio);
		m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_FR:
		m_eCurState = STATE_RUN_F;
		m_pTransformCom->Turn(_vLook, _vCamLook * (0.5f) + _vCamRight * 0.5f, _fRatio);
		m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_BR:
		m_eCurState = STATE_RUN_F;
		m_pTransformCom->Turn(_vLook, _vCamLook * (-0.5f) + _vCamRight * 0.5f, _fRatio);
		m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_FL:
		m_eCurState = STATE_RUN_F;
		m_pTransformCom->Turn(_vLook, _vCamLook * (0.5f) + _vCamRight * (-0.5f), _fRatio);
		m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_BL:
		m_eCurState = STATE_RUN_F;
		m_pTransformCom->Turn(_vLook, _vCamLook * (-0.5f) + _vCamRight * (-0.5f), _fRatio);
		m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_END:
		m_eCurState = STATE_IDLE;
		break;
	}
}

void CPlayer::TargetingMove(_float fTimeDelta)
{
	AUTOINSTANCE(CGameInstance, pGameInstance);
	if (pGameInstance->KeyPressing(DIK_A))
	{
		if (pGameInstance->KeyPressing(DIK_W))
		{
			m_eDir = DIR_FL;
		}
		else if (pGameInstance->KeyPressing(DIK_S))
		{
			m_eDir = DIR_BL;
		}
		else
		{
			m_eDir = DIR_L;
		}
	}
	else if (pGameInstance->KeyPressing(DIK_D))
	{
		if (pGameInstance->KeyPressing(DIK_W))
		{
			m_eDir = DIR_FR;
		}
		else if (pGameInstance->KeyPressing(DIK_S))
		{
			m_eDir = DIR_BR;
		}
		else
		{
			m_eDir = DIR_R;
		}
	}
	else
	{
		if (pGameInstance->KeyPressing(DIK_W))
		{
			m_eDir = DIR_F;
		}
		else if (pGameInstance->KeyPressing(DIK_S))
		{
			m_eDir = DIR_B;
		}
		else
		{
			m_eDir = DIR_END;
		}
	}

	_float4x4 _CamMatix;
	XMStoreFloat4x4(&_CamMatix, XMMatrixInverse(nullptr, pGameInstance->Get_TransformMatrix(CPipeLine::D3DTS_VIEW)));

	_vector	_vCamLook, _vCamRight, _vLook, _vRight;
	_float _fRatio = 0.8f;
	_vCamLook = XMLoadFloat4((_float4*)&_CamMatix.m[2][0]);
	_vCamRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), _vCamLook));
	_vCamLook = XMVector3Normalize(XMVector3Cross(_vCamRight, XMVectorSet(0.f, 1.f, 0.f, 0.f)));

	_vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	_vRight = m_pTransformCom->Get_State(CTransform::STATE_RIGHT);

	switch (m_eDir)
	{
	case Client::CPlayer::DIR_F:
		m_eCurState = STATE_RUN_F;
		m_pTransformCom->Turn(_vLook, _vCamLook, _fRatio);
		m_pTransformCom->Go_Straight(fTimeDelta, m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_B:
		m_eCurState = STATE_RUN_B;

		m_pTransformCom->Turn(_vLook, _vCamLook, _fRatio);
		m_pTransformCom->Go_Straight(-fTimeDelta, m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_R:
		m_eCurState = STATE_RUN_R;

		m_pTransformCom->Turn(_vLook, _vCamLook, _fRatio);
		m_pTransformCom->Go_Right(fTimeDelta, m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_L:
		m_eCurState = STATE_RUN_L;

		m_pTransformCom->Turn(_vLook, _vCamLook, _fRatio);
		m_pTransformCom->Go_Right(-fTimeDelta, m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_FR:
		m_eCurState = STATE_RUN_FR;
		m_pTransformCom->Turn(_vLook, _vCamLook, _fRatio);
		m_pTransformCom->Go_Straight((fTimeDelta) / (sqrtf(2.f)), m_pNavigationCom);
		m_pTransformCom->Go_Right((fTimeDelta) / (sqrtf(2.f)), m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_BR:
		m_eCurState = STATE_RUN_BR;
		m_pTransformCom->Turn(_vLook, _vCamLook, _fRatio);
		m_pTransformCom->Go_Straight((-fTimeDelta) / (sqrtf(2.f)), m_pNavigationCom);
		m_pTransformCom->Go_Right((fTimeDelta) / (sqrtf(2.f)), m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_FL:
		m_eCurState = STATE_RUN_FL;
		m_pTransformCom->Turn(_vLook, _vCamLook, _fRatio);
		m_pTransformCom->Go_Straight((fTimeDelta) / (sqrtf(2.f)), m_pNavigationCom);
		m_pTransformCom->Go_Right((-fTimeDelta) / (sqrtf(2.f)), m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_BL:
		m_eCurState = STATE_RUN_BL;
		m_pTransformCom->Turn(_vLook, _vCamLook, _fRatio);
		m_pTransformCom->Go_Straight((-fTimeDelta) / (sqrtf(2.f)), m_pNavigationCom);
		m_pTransformCom->Go_Right((-fTimeDelta) / (sqrtf(2.f)), m_pNavigationCom);
		break;
	case Client::CPlayer::DIR_END:
		m_eCurState = STATE_IDLE;
		break;
	}
}

void CPlayer::KP_ATT(_float fTimeDelta)
{
	//1. 리미트 시간 전에 눌렀을때 반응하는 상태 : 다음 공격키(예약)
	//2. 리미트 시간 후에 눌렀을때 반응하는 상태 : 패리(즉각)
	//3. 아무때나 눌렀을때 반응하는 상태 : 클로(예약), 회피(즉각)
	AUTOINSTANCE(CGameInstance, pGameInstance);
	switch (m_eCurState)
	{
	case Client::CPlayer::STATE_ATT1:
		if (m_fPlayTime <= m_vecLimitTime[STATE_ATT1][ATTACKLIMIT_CHANGE])
		{
			//if (CGameInstance::Get_Instance()->KeyDown(DIK_SPACE))
			if (pGameInstance->MouseDown(DIMK_LBUTTON))
			{
				m_eReserveState = STATE_ATT2;
			}
		}
		if (m_fPlayTime > m_vecLimitTime[STATE_ATT1][ATTACKLIMIT_CHANGE])
		{
			if (pGameInstance->KeyDown(DIK_F))
			{
				m_eCurState = ParryL;
			}
		}
		break;
	case Client::CPlayer::STATE_ATT2:
		if (m_fPlayTime <= m_vecLimitTime[STATE_ATT2][ATTACKLIMIT_CHANGE])
		{
			//if (CGameInstance::Get_Instance()->KeyDown(DIK_SPACE))
			if (pGameInstance->MouseDown(DIMK_LBUTTON))
			{
				m_eReserveState = STATE_ATT3;
			}

		}
		if (m_fPlayTime > m_vecLimitTime[STATE_ATT2][ATTACKLIMIT_CHANGE])
		{
			if (pGameInstance->KeyDown(DIK_F))
			{
				m_eReserveState = ParryL;
			}

		}
		break;
	case Client::CPlayer::STATE_ATT3:
		if (m_fPlayTime <= m_vecLimitTime[STATE_ATT3][ATTACKLIMIT_CHANGE])
		{
			//if (CGameInstance::Get_Instance()->KeyDown(DIK_SPACE))
			if (pGameInstance->MouseDown(DIMK_LBUTTON))
			{
				m_eReserveState = STATE_ATT4;
			}
		}
		if (m_fPlayTime > m_vecLimitTime[STATE_ATT3][ATTACKLIMIT_CHANGE])
		{
			if (pGameInstance->KeyDown(DIK_F))
			{
				m_eReserveState = ParryL;
			}

		}
		break;
	case Client::CPlayer::STATE_ATT4:
		if (m_fPlayTime <= m_vecLimitTime[STATE_ATT4][ATTACKLIMIT_CHANGE])
		{
			//if (CGameInstance::Get_Instance()->KeyDown(DIK_SPACE))
			if (pGameInstance->MouseDown(DIMK_LBUTTON))
			{
				m_eReserveState = STATE_ATT5;
			}
		}
		if (m_fPlayTime > m_vecLimitTime[STATE_ATT4][ATTACKLIMIT_CHANGE])
		{
			if (pGameInstance->KeyDown(DIK_F))
			{
				m_eReserveState = ParryL;
			}
		}
		break;
	case Client::CPlayer::STATE_ATT5:
		if (m_fPlayTime > m_vecLimitTime[STATE_ATT5][ATTACKLIMIT_CHANGE])
		{
			if (pGameInstance->KeyDown(DIK_F))
			{
				m_eReserveState = ParryL;
			}
		}
		break;
	}
	if (pGameInstance->MouseDown(DIMK_RBUTTON))
	{
		m_eReserveState = Raven_ClawNear;

	}
	if (pGameInstance->KeyDown(DIK_SPACE))
	{
		m_eCurState = STATE_AVOIDATTACK;
		if (pGameInstance->KeyPressing(DIK_S))
		{
			m_eCurState = STATE_AVOIDBACK;
		}
	}
}

void CPlayer::KP_Parry(_float fTimeDelta)
{

}

void CPlayer::KP_AVOIDATTACK(_float fTimeDelta)
{
	AUTOINSTANCE(CGameInstance, pGameInstance);
	if (5.f < m_fPlayTime)
	{
		if (pGameInstance->KeyDown(DIK_SPACE))
		{
			m_eCurState = STATE_AVOIDATTACK;
			if (pGameInstance->KeyPressing(DIK_S))
			{
				m_eCurState = STATE_AVOIDBACK;
				m_eWeapon = WEAPON_BASE;
			}
			if (m_bAgainAnim == false)
			{
				m_bAgainAnim = true;
				Set_Anim(m_eCurState);
			}
		}
	}
}

void CPlayer::KP_ClawNear(_float fTimeDelta)
{
	AUTOINSTANCE(CGameInstance, pGameInstance);
	if (pGameInstance->KeyDown(DIK_SPACE))
	{
		m_eCurState = STATE_AVOIDATTACK;
	}
}

void CPlayer::Targeting()
{
	/*if (m_pTarget != nullptr)
	{
	_vector _vTargetPos = static_cast<CTransform*>(m_pTarget->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION);
	m_pTransformCom->LookAt_ForLandObject(_vTargetPos);
	}*/
	if (m_pTarget == nullptr)
	{
		list<CGameObject*> Monsters = *m_MonsterLayer->Get_ListFromLayer();

		_vector _vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		_float _fClosedDis(35.f);
		for (auto iter : Monsters)
		{
			if (static_cast<CMonster*>(iter)->Get_MonsterState() == CMonster::ATTACK_DEAD)
				continue;

			_float _fDis = fabs(XMVectorGetX(XMVector3Length(static_cast<CTransform*>(iter->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION)
				- _vPos)));
			if (_fDis < 35.f)
			{
				if (_fClosedDis > _fDis)
				{
					m_pTarget = iter;
					_fClosedDis = _fDis;
				}
			}
		}
		if (m_pTarget != nullptr)
		{
			Safe_AddRef(m_pTarget);
			static_cast<CCamera_Player*>(CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER))->Get_Target(static_cast<CTransform*>(m_pTarget->Get_ComponentPtr(TEXT("Com_Transform"))));
		}
	}
	else
	{
		Safe_Release(m_pTarget);
		static_cast<CCamera_Player*>(CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER))->Get_Target(nullptr);
	}

}

void CPlayer::TargetCheck()
{
	//아직 거리 안줬넹 ㅎ헤ㅔㅔ
	if (static_cast<CMonster*>(m_pTarget)->Get_MonsterState() == CMonster::ATTACK_DISAPPEAR || static_cast<CMonster*>(m_pTarget)->Get_MonsterState() == CMonster::ATTACK_DEAD)
	{
		Safe_Release(m_pTarget);
		static_cast<CCamera_Player*>(CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER))->Get_Target(nullptr);
	}
	else if (35.f<XMVectorGetX(XMVector3Length(static_cast<CTransform*>(m_pTarget->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION) - m_pTransformCom->Get_State(CTransform::STATE_POSITION))))
	{
		Safe_Release(m_pTarget);
		static_cast<CCamera_Player*>(CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER))->Get_Target(nullptr);
	}

}

void CPlayer::CutScene()
{
	AUTOINSTANCE(CGameInstance, _pInstance);
	//388
	if (g_eCurLevel == LEVEL_GAMEPLAY && !m_bCutScene[LEVEL_GAMEPLAY])
	{
		if (m_pNavigationCom->Get_Index() == 6)
		{
			AUTOINSTANCE(CCameraMgr, _pCamera);
			m_eCurState = Corvus_VS_MagicianLV1_Seq_BossFightStart;
			m_pModelCom->DirectAnim(Corvus_VS_MagicianLV1_Seq_BossFightStart);
			//m_pTransformCom->Turn_Angle(XMVectorSet(0.f,1.f,0.f,0.f),XMConvertToRadians(180.f));

			CMagician* _pMagician = dynamic_cast<CMagician*>(_pInstance->Get_Layer(LEVEL_GAMEPLAY, TEXT("Layer_Monster"))->Get_ObjFromLayer(0));
			_pMagician->Set_AnimState(CMagician::Boss_Enter);
			CTransform* _TargetTrans = static_cast<CTransform*>(_pMagician->Get_ComponentPtr(TEXT("Com_Transform")));
			CAnimModel* _pModel = static_cast<CAnimModel*>(_pMagician->Get_ComponentPtr(TEXT("Com_Model")));
			_pModel->DirectAnim(CMagician::Boss_Enter);
			
			
			_vector vTargetPos = (m_pModelCom->Get_HierarchyNode("AnimTargetPoint")->Get_CombinedTransformation()
				*XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())*m_pTransformCom->Get_WorldMatrix()).r[3];
			/*vTargetPos.m128_f32[0] *= 0.01f;
			vTargetPos.m128_f32[1] *= 0.01f;
			vTargetPos.m128_f32[2] *= 0.01f;*/
			_TargetTrans->Set_State(CTransform::STATE_POSITION, vTargetPos);
			m_pTransformCom->LookAt_ForLandObject(_TargetTrans->Get_State(CTransform::STATE_POSITION));
			m_pTransformCom->Turn_Angle(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
			_TargetTrans->LookAt_ForLandObject(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
			_pCamera->Change_Camera(CCameraMgr::CAMERA_CUTSCENE_ENTER);
			CGameObject* _pCard = _pInstance->Clone_GameObject(TEXT("Prototype_GameObject_Weapon_Card"));
			//CTransform* CardTrans = static_cast<CTransform*>(_pCard->Get_ComponentPtr(TEXT("Com_Transform")));
			static_cast<CCamera_CutScene_Enter*>(_pCamera->Get_Cam(CCameraMgr::CAMERA_CUTSCENE_ENTER))->Set_Target(_pMagician);
			//_pCamera->Get_Cam(CCameraMgr::CAMERA_CUTSCENE_ENTER)->ZoomIn(30.f, 200.f);
			static_cast<CCamera_CutScene_Enter*>(_pCamera->Get_Cam(CCameraMgr::CAMERA_CUTSCENE_ENTER))->Set_Trans(vTargetPos);
			_pMagician->Set_Card(_pCard);
			m_bCutScene[LEVEL_GAMEPLAY] = true;
			m_bRender = false;
		}
	}
	else if (g_eCurLevel == LEVEL_STAGE_LAST && !m_bCutScene[LEVEL_STAGE_LAST])
	{
		if (m_pNavigationCom->Get_Index() == 776)
		{
			AUTOINSTANCE(CGameInstance, pGameInstance);
			AUTOINSTANCE(CCameraMgr, _pCamera);
			_pCamera->Change_Camera(CCameraMgr::CAMERA_CUTSCENE);
			m_eCurState = Corvus_VS_Puppet_Execution_Start;
			m_pModelCom->DirectAnim(Corvus_VS_Puppet_Execution_Start);
			_pCamera->Get_Cam(CCameraMgr::CAMERA_CUTSCENE)->Set_FOV(60.f);
			static_cast<CCamera_CutScene*>(_pCamera->Get_Cam(CCameraMgr::CAMERA_CUTSCENE))->LookAt("Bip001-Pelvis");
			m_bCutScene[LEVEL_STAGE_LAST] = true;
			_pInstance->Set_TimeSpeed(TEXT("Timer_Main"), 1.f);
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(48.768f, 41.997f, 83.445f, 1.f));
			m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(-104.f));

			POINTLIGHTDESC			LightDesc;
			ZeroMemory(&LightDesc, sizeof(DIRLIGHTDESC));

			XMStoreFloat4(&LightDesc.vPosition, XMVectorSet(48.889f, 41.978f, 83.506f, 1.f));
			LightDesc.fRange = 15.f;
			LightDesc.vDiffuse = CLIENT_RGB(255.f, 255.f, 255.f);
			LightDesc.vAmbient = _float4(0.5f, 0.5f, 0.5f, 1.0f);
			LightDesc.vSpecular = _float4(0.f, 0.f, 0.f, 1.f);
			LightDesc.LightProjMatrix = new _float4x4;
			LightDesc.LightViewMatrix = new _float4x4;

			XMStoreFloat4x4(LightDesc.LightViewMatrix, XMMatrixIdentity());
			XMStoreFloat4x4(LightDesc.LightProjMatrix, XMMatrixIdentity());

			m_iLightIndex = pGameInstance->Add_Light(m_pDevice, m_pContext, LEVEL_STAGE_LAST, CLight_Manager::STATICPOINTLIHGT, LightDesc);

			if (m_iLightIndex == -1)
				return;


			pGameInstance->Light_On(LEVEL_STAGE_LAST, CLight_Manager::STATICPOINTLIHGT, m_iLightIndex);
			CTransform* _TargetTrans = static_cast<CTransform*>(m_MonsterLayer->Get_ObjFromLayer(0)->Get_ComponentPtr(TEXT("Com_Transform")));
			m_pTransformCom->LookAt_ForLandObject(
				XMVectorSet(45.f,-25.f,45.f, 1.f));
			_TargetTrans->Set_State(CTransform::STATE_POSITION, XMVectorSet(55.528f, -23.2f, 56.937f, 1.f));
			_TargetTrans->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(0.f));
			static_cast<CPuppet*>(m_MonsterLayer->Get_ObjFromLayer(0))->Set_PuppetEnd(true);
		}
	}
}

void CPlayer::Gate()
{
	if (m_pNavigationCom->Get_Index() == 71 || m_pNavigationCom->Get_Index() == 70)
	{
		m_eCurState = STATE_APPROACH;
	}
	else if (m_pNavigationCom->Get_Index() == 131 || m_pNavigationCom->Get_Index() == 132)
	{
		m_eCurState = STATE_APPROACH;
	}
	else if (m_pNavigationCom->Get_Index() == 129 || m_pNavigationCom->Get_Index() == 130)
	{
		m_eCurState = STATE_APPROACH;
	}
}

void CPlayer::MoveLevel()
{
	AUTOINSTANCE(CGameInstance, _pInstance);
	if (g_eCurLevel == LEVEL_STAGE_LOBBY)
	{
		//클리어 스테이지에 따라 if문처리

		switch (m_iStage)
		{
		case 0:
			_pInstance->Reserve_Level(LEVEL_GAMEPLAY);
			break;
		case 1:
			_pInstance->Reserve_Level(LEVEL_STAGE_02_1);
			break;
		case 2:
			_pInstance->Reserve_Level(LEVEL_STAGE_LAST);
			break;
		}
	}
}

void CPlayer::CheckEndAnim()
{
	AUTOINSTANCE(CCameraMgr, _pCamera);
	AUTOINSTANCE(CGameInstance, _pInstance);
	switch (m_eCurState)
	{
	case Client::CPlayer::STATE_ATT1:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_ATT2:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_ATT3:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_ATT4:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_ATT5:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_RUN_B:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_RUN_F:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_RUN_L:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_RUN_R:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_APPROACH:
		MoveLevel();
		m_eCurState = STATE_IDLE;
		m_eWeapon = WEAPON_BASE;
		break;
	case Client::CPlayer::STATE_WALK:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_IDLE:
		break;
	case Client::CPlayer::STATE_AVOIDATTACK:
		m_bCollision[COLLIDERTYPE_BODY] = true;
		m_iHitCount = 0;
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_JUMPAVOID:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_AVOIDBACK:
		m_bCollision[COLLIDERTYPE_BODY] = true;
		m_iHitCount = 0;
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::Corvus_PW_Axe:
		m_bCollision[COLLIDERTYPE_BODY] = true;
		m_eCurState = STATE_IDLE;
		m_bMotionPlay = false;
		break;
	case Client::CPlayer::Tackle_F:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::ParryR:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::ParryL:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::DualKnife:
		m_eCurState = STATE_IDLE;
		m_eWeapon = WEAPON_BASE;
		m_pSkillParts[SKILL_DUAL][0]->End();
		m_pSkillParts[SKILL_DUAL][1]->End();
		break;
	case Client::CPlayer::GreatSword:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::PW_Halberds:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::PW_Hammer_A:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::PW_TwinSwords_1:
		m_eCurState = PW_TwinSwords_2;
		break;
	case Client::CPlayer::PW_TwinSwords_2:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::PW_VargSword_A:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::PW_Bow_Start:
		m_eWeapon = WEAPON_BASE;
		m_eCurSkill = SKILL_END;
		m_eCurState = STATE_IDLE;
		m_pSkillParts[SKILL_BOW][0]->End();
		break;
	case Client::CPlayer::PW_Bow_End:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::PW_Bow_B:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::PW_Bloodwhip:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::PW_CaneSword_SP01:
		m_eCurState = PW_CaneSword_SP02;
		break;
	case Client::CPlayer::PW_CaneSword_SP02:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::Healing_Little:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::Healing2_Blend:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::Raven_ClawCommonV2_ChargeStart:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::Raven_ClawLong_ChargeFull:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::Raven_ClawNear:
		m_eWeapon = WEAPON_BASE;
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::Strong_Jump:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::RavenAttack_Start:
		m_eCurState = RavenAttack_End;
		break;
	case Client::CPlayer::RavenAttack_End:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::SD_ParryToMob:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::SD_HurtIdle:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::SD_StrongHurt_Start:
		m_eCurState = SD_StrongHurt_End;
		break;
	case Client::CPlayer::SD_StrongHurt_End:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::SD_Dead:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_RUN_BL:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_RUN_BR:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_RUN_FL:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::STATE_RUN_FR:
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::Corvus_VSLV1Villager_M_Execution:
		if (m_bOneChange)
		{
			Change_Hand();
			m_bOneChange = false;
			_pInstance->Set_TimeSpeed(TEXT("Timer_Main"), DEFAULTTIME);
			_pCamera->Get_Cam(CCameraMgr::CAMERA_PLAYER)->ZoomOff(100.f);
		}
		m_eCurState = STATE_IDLE;
		
		break;
	case Client::CPlayer::Corvus_VSLV2Villager_M_Execution:
		if (m_bOneChange)
		{
			Change_Hand();
			_pInstance->Set_TimeSpeed(TEXT("Timer_Main"), DEFAULTTIME);
			_pCamera->Get_Cam(CCameraMgr::CAMERA_PLAYER)->ZoomOff(100.f);
			m_bOneChange = false;
			//m_eWeapon = WEAPON_BASE;
		}
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::Corvus_VSMagician_Execution:
		_pCamera->Change_Camera(CCameraMgr::CAMERA_PLAYER);
		_pInstance->Set_TimeSpeed(TEXT("Timer_Main"), DEFAULTTIME);
		m_eCurState = STATE_IDLE;
		m_iStage = 1;
		break;
	case Client::CPlayer::Corvus_VSBossBat_Execution01:
		_pCamera->Change_Camera(CCameraMgr::CAMERA_PLAYER);
		m_eCurState = STATE_IDLE;
		m_iStage = 2;
		break;
	case Client::CPlayer::STATE_APPROACH2:
		Safe_Release(m_pNavigationCom);
		m_eCurState = Strong_Jump2;
		m_eWeapon = WEAPON_NONE;
		break;
	case Client::CPlayer::Strong_Jump2:
		_pInstance->Set_TimeSpeed(TEXT("Timer_Main"), DEFAULTTIME);
		static_cast<CCamera_CutScene_Enter*>(_pCamera->Get_Cam(CCameraMgr::CAMERA_CUTSCENE_ENTER))->Set_CutSceneNum(5);

		m_eCurState = Corvus_SD_Fall_Loop;
		break;
	case Client::CPlayer::Corvus_SD_Fall_Loop:
		if (g_eCurLevel == LEVEL_STAGE_02_1)
			_pInstance->Reserve_Level(LEVEL_STAGE_02);
		else if (g_eCurLevel == LEVEL_STAGE_02)
			m_eCurState = Corvus_SD_Fall_Loop;
		
		break;
	case Client::CPlayer::Corvus_SD_Fall_End:
		if (g_eCurLevel == LEVEL_STAGE_02)
		{
			CNavigation::NAVIGATIONDESC			NaviDesc;
			ZeroMemory(&NaviDesc, sizeof(CNavigation::NAVIGATIONDESC));
			NaviDesc.iCurrentIndex = 0;
			if (FAILED(__super::Add_Component(LEVEL_STAGE_02, TEXT("Prototype_Component_Navigation_Stage_02"), TEXT("Com_Navigation"), (CComponent**)&m_pNavigationCom, &NaviDesc)))
				return;
			m_eCurState = STATE_IDLE;
			static_cast<CCamera_CutScene_Enter*>(_pCamera->Get_Cam(CCameraMgr::CAMERA_CUTSCENE_ENTER))->Set_CutSceneNum(1);
			m_eWeapon = WEAPON_BASE;
			_pInstance->Set_TimeSpeed(TEXT("Timer_Main"), DEFAULTTIME);
		}
		
		break;
	case Client::CPlayer::Corvus_VS_MagicianLV1_Seq_BossFightStart:
		_pCamera->Change_Camera(CCameraMgr::CAMERA_PLAYER);
		m_eCurState = STATE_IDLE;
		m_pTransformCom->Turn_Angle(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
		m_pModelCom->DirectAnim(STATE_IDLE);
		
		break;
	case Client::CPlayer::Corvus_VS_Puppet_Execution_Start:
		m_eCurState = Corvus_VS_Puppet_Execution_Attack;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(56.234f, 37.997f, 54.109f, 1.f));
		m_pTransformCom->Turn_Angle(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
		static_cast<CPuppet*>(m_MonsterLayer->Get_ObjFromLayer(0))->Set_AnimState(CPuppet::Puppet_Idle_F);
		static_cast<CPuppet*>(m_MonsterLayer->Get_ObjFromLayer(0))->Set_PuppetEnd(true);
		static_cast<CTransform*>(m_MonsterLayer->Get_ObjFromLayer(0)->Get_ComponentPtr(TEXT("Com_Transform")))->Set_State(CTransform::STATE_POSITION, XMVectorSet(55.f, -48.f, 57.f, 1.f));

		static_cast<CAnimModel*>(m_MonsterLayer->Get_ObjFromLayer(0)->Get_ComponentPtr(TEXT("Com_Model")))->DirectAnim(CPuppet::Puppet_Idle_F);

		break;
	case Client::CPlayer::Corvus_VS_Puppet_Execution_Attack:
	{
		m_eCurState = Corvus_VS_Puppet_Execution_End;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(46.269f, 41.978f, 77.663f, 1.f));
		static_cast<CPuppet*>(m_MonsterLayer->Get_ObjFromLayer(0))->Set_PuppetEnd(false);
		m_pTransformCom->Turn_Angle(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
		static_cast<CPuppet*>(m_MonsterLayer->Get_ObjFromLayer(0))->Set_AnimState(CPuppet::Puppet_VS_TakeExecution_End);
		static_cast<CTransform*>(m_MonsterLayer->Get_ObjFromLayer(0)->Get_ComponentPtr(TEXT("Com_Transform")))->Set_State(CTransform::STATE_POSITION, XMVectorSet(48.f, -23.f, 17.f, 1.f));

		
		static_cast<CAnimModel*>(m_MonsterLayer->Get_ObjFromLayer(0)->Get_ComponentPtr(TEXT("Com_Model")))->DirectAnim(CPuppet::Puppet_VS_TakeExecution_End);
		
	}
		
		break;
	case Client::CPlayer::Corvus_VS_Puppet_Execution_End:
		_pCamera->Change_Camera(CCameraMgr::CAMERA_PLAYER);
		//CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER)->Shake_On(3.f, 2.f);

		//_pInstance->Light_Off(LEVEL_STAGE_LAST, CLight_Manager::STATICPOINTLIHGT, m_iLightIndex);
		m_eCurState = STATE_IDLE;
		break;
	case Client::CPlayer::Corvus_Raven_ClawLong_ChargeFull:
		m_eCurState = Corvus_Raven_ClawLong_PlunderAttack2TTT;
		
		break;
	case Client::CPlayer::Corvus_Raven_ClawLong_PlunderAttack2TTT:
		m_eWeapon = WEAPON_BASE;
		m_eCurState = STATE_IDLE;
		for (_uint i = 0; i < NAIL_END; ++i)
		{
			Safe_Release(m_pClawTrail[i]);
		}
		break;
	case Client::CPlayer::Corvus_PW_Scythe:
		m_eCurState = STATE_IDLE;
		m_eWeapon = WEAPON_BASE;
		if (m_pSkillParts[SKILL_SCYTHE][0]->Trail_GetOn())
			m_pSkillParts[SKILL_SCYTHE][0]->TrailOff();
		m_pSkillParts[SKILL_SCYTHE][0]->End();
		break;
	case Client::CPlayer::Corvus_PW_Halberds:
		m_eCurState = STATE_IDLE;
		m_eWeapon = WEAPON_BASE;
		if (m_pSkillParts[SKILL_LENCE][0]->Trail_GetOn())
			m_pSkillParts[SKILL_LENCE][0]->TrailOff();
		m_pSkillParts[SKILL_LENCE][0]->End();
		for (auto& _pEffect : m_vecWind)
			Safe_Release(_pEffect);
		m_vecWind.clear();
		break;
	}


	XMStoreFloat4(&m_AnimPos, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	m_PreAnimPos = m_AnimPos;
	m_bEffect = false;
	m_fUseStealSkill = 1.f;
}

void CPlayer::CheckLimit()
{
	switch (m_eCurState)
	{
	case Client::CPlayer::STATE_ATT1:
		if (CheckLimit_Att(STATE_ATT1))
			return;
		break;
	case Client::CPlayer::STATE_ATT2:
		if (CheckLimit_Att(STATE_ATT2))
			return;
		break;
	case Client::CPlayer::STATE_ATT3:
		if (CheckLimit_Att(STATE_ATT3))
			return;
		break;
	case Client::CPlayer::STATE_ATT4:
		if (CheckLimit_Att(STATE_ATT4))
			return;
		break;
	case Client::CPlayer::STATE_ATT5:
		if (CheckLimit_Att(STATE_ATT5))
			return;
		break;
	case Client::CPlayer::STATE_RUN_B:
		break;
	case Client::CPlayer::STATE_RUN_F:
		break;
	case Client::CPlayer::STATE_RUN_L:
		break;
	case Client::CPlayer::STATE_RUN_R:
		break;
	case Client::CPlayer::STATE_APPROACH:
		break;
	case Client::CPlayer::STATE_IDLE:
		break;
	case Client::CPlayer::STATE_WALK:
		break;
	case Client::CPlayer::STATE_AVOIDATTACK:
		if (m_fPlayTime > m_vecLimitTime[STATE_AVOIDATTACK][0])
		{

		}
		break;
	case Client::CPlayer::Corvus_PW_Axe:
		if (m_fPlayTime > m_vecLimitTime[Corvus_PW_Axe][6])//다시 무기 스왑 및 타이머 정상화
		{
			m_eWeapon = WEAPON_BASE;
			m_eCurSkill = SKILL_END;
			AUTOINSTANCE(CGameInstance, pGame);
			
			pGame->Set_TimeSpeed(TEXT("Timer_Main"), DEFAULTTIME);
			if (!m_bEffect)
			{
				m_pSkillParts[SKILL_AXE][0]->End();
				m_bEffect = true;
			}
		}
		else if (m_fPlayTime > m_vecLimitTime[Corvus_PW_Axe][5])//다시 무기 스왑 및 타이머 정상화
		{
			if (m_pSkillParts[SKILL_AXE][0]->Trail_GetOn())
				m_pSkillParts[SKILL_AXE][0]->TrailOff();
			
			
			AUTOINSTANCE(CGameInstance, pGame);
			pGame->Set_TimeSpeed(TEXT("Timer_Main"), 2.f);
		}
		else if (m_fPlayTime > m_vecLimitTime[Corvus_PW_Axe][4])//다시 무기 스왑 및 타이머 정상화
		{
			m_pSkillParts[SKILL_AXE][0]->Set_CollisionOn(false);
			m_pStatusCom->Set_Attack(30.f);
		}
		else if (m_fPlayTime > m_vecLimitTime[Corvus_PW_Axe][3])//모션트레일 off
		{
			m_pSkillParts[SKILL_AXE][0]->Set_CollisionOn(true);
			m_pStatusCom->Set_Attack(m_fSkillAtts[SKILL_AXE] * m_fUseStealSkill);
			m_bMotionPlay = false;
		}
		else if (m_fPlayTime > m_vecLimitTime[Corvus_PW_Axe][2])//모션트레일 off
		{
			if (!m_pSkillParts[SKILL_AXE][0]->Trail_GetOn())
				m_pSkillParts[SKILL_AXE][0]->TrailOn();

		}
		else if (m_fPlayTime > m_vecLimitTime[Corvus_PW_Axe][1])//타이머 및 모션트레일
		{
			
			
			m_bMotionPlay = true;
			AUTOINSTANCE(CGameInstance, pGame);
			pGame->Set_TimeSpeed(TEXT("Timer_Main"), 0.5f);
			m_eCurSkill = SKILL_AXE;
			m_eWeapon = WEAPON_SKILL;
		}
		else if (m_fPlayTime > m_vecLimitTime[Corvus_PW_Axe][0])//무기 스왑
		{
			m_eCurSkill = SKILL_AXE;
			m_eWeapon = WEAPON_SKILL;
		}
		break;
	case Client::CPlayer::Tackle_F:
		break;
	case Client::CPlayer::ParryR:
		if (m_fPlayTime > m_vecLimitTime[ParryL][1])//무기 스왑
		{
			m_bCollision[COLLIDERTYPE_PARRY] = false;
			//m_pBaseParts[BASE_DAGGER]->Set_CollisionOn(false);
		}
		else if (m_fPlayTime > m_vecLimitTime[ParryL][0])//무기 스왑
		{
			m_bCollision[COLLIDERTYPE_PARRY] = true;
			//m_pBaseParts[BASE_DAGGER]->Set_CollisionOn(true);
		}
		break;
	case Client::CPlayer::ParryL:
		if (m_fPlayTime > m_vecLimitTime[ParryL][1])//무기 스왑
		{
			m_bCollision[COLLIDERTYPE_PARRY] = false;
			//m_pBaseParts[BASE_DAGGER]->Set_CollisionOn(false);
		}
		else if (m_fPlayTime > m_vecLimitTime[ParryL][0])//무기 스왑
		{
			m_bCollision[COLLIDERTYPE_PARRY] = true;
			//m_pBaseParts[BASE_DAGGER]->Set_CollisionOn(true);
		}
		break;
	case Client::CPlayer::DualKnife:
		if (m_fPlayTime > m_vecLimitTime[DualKnife][1])//무기 스왑
		{
			m_pSkillParts[SKILL_DUAL][HAND_LEFT]->Set_CollisionOn(false);
			m_pSkillParts[SKILL_DUAL][HAND_RIGHT]->Set_CollisionOn(false);
			m_pStatusCom->Set_Attack(30.f);
			if (m_pSkillParts[SKILL_DUAL][HAND_LEFT]->Trail_GetOn())
				m_pSkillParts[SKILL_DUAL][HAND_LEFT]->TrailOff();
			if (m_pSkillParts[SKILL_DUAL][HAND_RIGHT]->Trail_GetOn())
				m_pSkillParts[SKILL_DUAL][HAND_RIGHT]->TrailOff();
		}
		else if (m_fPlayTime > m_vecLimitTime[DualKnife][0])
		{
			if (!m_pSkillParts[SKILL_DUAL][HAND_LEFT]->Trail_GetOn())
				m_pSkillParts[SKILL_DUAL][HAND_LEFT]->TrailOn();
			if (!m_pSkillParts[SKILL_DUAL][HAND_RIGHT]->Trail_GetOn())
				m_pSkillParts[SKILL_DUAL][HAND_RIGHT]->TrailOn();
			m_pSkillParts[SKILL_DUAL][HAND_LEFT]->Set_CollisionOn(true);
			m_pStatusCom->Set_Attack(m_fSkillAtts[SKILL_DUAL] * m_fUseStealSkill);
			m_pSkillParts[SKILL_DUAL][HAND_RIGHT]->Set_CollisionOn(true);
		}
		break;
	case Client::CPlayer::GreatSword:
		break;
	case Client::CPlayer::PW_Halberds:
		break;
	case Client::CPlayer::PW_Hammer_A:
		break;
	case Client::CPlayer::PW_TwinSwords_1:
		break;
	case Client::CPlayer::PW_TwinSwords_2:
		break;
	case Client::CPlayer::PW_VargSword_A:
		break;
	case Client::CPlayer::PW_Bow_Start:
		if (m_fPlayTime > m_vecLimitTime[PW_Bow_Start][0]) // shoot
		{
			if (!m_bEffect)
			{
				if (m_pTarget)
					m_pTransformCom->LookAt_ForLandObject(static_cast<CTransform*>(m_pTarget->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION));
				//static_cast<CBow*>(m_pSkillParts[SKILL_BOW][0])->Shoot();

				CArrow::ARROW _tArrow;
				_tArrow.fUseStealSkill = m_fUseStealSkill;
				XMStoreFloat4x4(&_tArrow.StartMatrix, /*m_pTransformCom->Get_WorldMatrix() * */m_pTransformCom->Get_WorldMatrix());
				CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_ARROW, &_tArrow);

				m_bEffect = true;
			}
		}
		break;
	case Client::CPlayer::PW_Bow_End:
		break;
	case Client::CPlayer::PW_Bow_B:
		break;
	case Client::CPlayer::PW_Bloodwhip:
		break;
	case Client::CPlayer::PW_CaneSword_SP01:
		break;
	case Client::CPlayer::PW_CaneSword_SP02:
		break;
	case Client::CPlayer::Healing_Little:
		break;
	case Client::CPlayer::Healing2_Blend:
		break;
	case Client::CPlayer::Raven_ClawCommonV2_ChargeStart:
		break;
	case Client::CPlayer::Raven_ClawLong_ChargeFull:
		break;
	case Client::CPlayer::Raven_ClawNear:
		if (m_fPlayTime > m_vecLimitTime[Raven_ClawNear][3])
		{
			
			m_bCollision[COLLIDERTYPE_BODY] = true;
			m_bCollision[COLLIDERTYPE_CLAW] = false;
		}
		else if (m_fPlayTime > m_vecLimitTime[Raven_ClawNear][2])
		{
			if (!m_bEffect)
			{
				CClaw::CLAW_DESC _tInfo;
				_tInfo.TargetMatrix = m_pTransformCom->Get_WorldFloat4x4();
				_tInfo.eClaw = CClaw::CLAWTYPE_LEFT;
				CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_CLAW, &_tInfo);

				m_bEffect = true;
			}
		}
		else if (m_fPlayTime > m_vecLimitTime[Raven_ClawNear][1])
		{
			
		}
		else if (m_fPlayTime > m_vecLimitTime[Raven_ClawNear][0])
		{
			m_bCollision[COLLIDERTYPE_BODY] = false;
			m_bCollision[COLLIDERTYPE_CLAW] = true;
		}
		break;
	case Client::CPlayer::Strong_Jump2:
		if (m_vecLimitTime[Strong_Jump2][0] < m_fPlayTime)
		{
			AUTOINSTANCE(CCameraMgr, _pCamera);
			AUTOINSTANCE(CGameInstance, _pInstance);

			static_cast<CCamera_CutScene_Enter*>(_pCamera->Get_Cam(CCameraMgr::CAMERA_CUTSCENE_ENTER))->Set_CutSceneNum(4);
			_pInstance->Set_TimeSpeed(TEXT("Timer_Main"), 0.7f);
		}
		else
		{
			AUTOINSTANCE(CCameraMgr, _pCamera);
			static_cast<CCamera_CutScene_Enter*>(_pCamera->Get_Cam(CCameraMgr::CAMERA_CUTSCENE_ENTER))->Set_CutSceneNum(2);
		}
		break;
	case Client::CPlayer::RavenAttack_Start:
		break;
	case Client::CPlayer::RavenAttack_End:
		break;
	case Client::CPlayer::SD_ParryToMob:
		break;
	case Client::CPlayer::SD_HurtIdle:
		if (m_fPlayTime > 15.f)
		{
			m_bCollision[COLLIDERTYPE_BODY] = true;
			//m_eWeapon = WEAPON_BASE;
		}
		else if (m_fPlayTime > 0.f)
		{
			//m_eWeapon = WEAPON_NONE;
			m_bCollision[COLLIDERTYPE_PARRY] = false;
		}
		break;
	case Client::CPlayer::SD_StrongHurt_Start:
		if (m_fPlayTime > 5.f)
		{
			//m_bCollision[COLLIDERTYPE_BODY] = true;
			//m_eWeapon = WEAPON_BASE;
		}
		else if (m_fPlayTime > 0.f)
		{
			//m_eWeapon = WEAPON_NONE;
			m_bCollision[COLLIDERTYPE_PARRY] = false;
		}
		break;
	case Client::CPlayer::SD_StrongHurt_End:
		break;
	case Client::CPlayer::SD_Dead:
		break;
	case Client::CPlayer::STATE_END:
		break;
	case Client::CPlayer::Corvus_Raven_ClawLong_ChargeFull:
		if (m_eReserveState == Corvus_Raven_ClawLong_PlunderAttack2TTT)
		{
			m_eReserveState = STATE_END;
			m_eCurState = Corvus_Raven_ClawLong_PlunderAttack2TTT;
		}
		break;
		
	case Client::CPlayer::Corvus_Raven_ClawLong_PlunderAttack2TTT:
		if (m_fPlayTime > 70.f)
		{
			AUTOINSTANCE(CGameInstance, _pInstance);
			m_bCollision[COLLIDERTYPE_BODY] = true;
			m_bCollision[COLLIDERTYPE_CLAW] = false;
			POINTLIGHTDESC* LIGHT;
			_pInstance->Light_On(g_eCurLevel, CLight_Manager::DYNAMICPOINTLIHGT, m_iClawLight);
			LIGHT = _pInstance->Get_PointLightDesc(g_eCurLevel, CLight_Manager::DYNAMICPOINTLIHGT, m_iClawLight);

			XMStoreFloat4(&(LIGHT->vPosition), XMVectorSetW(XMLoadFloat3(&m_vCurClaw), 1.f));
			LIGHT->fRange = 2.f;
		}
		else if (m_fPlayTime > 50.f)
		{
			AUTOINSTANCE(CGameInstance, _pInstance);
			Update_Nail();
			m_Particles[PARTICLE_STEAL].Center = m_vCurClaw;
			m_Particles[PARTICLE_STEAL].vColor = CLIENT_RGB(189.f, 245.f, 90.f);
			m_Particles[PARTICLE_STEAL].iNumParticles = 10;
			if (nullptr == CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_PARTICLE, &m_Particles[PARTICLE_STEAL]))
			{
				MSG_BOX(TEXT("fail effect"));
			}
			m_Particles[PARTICLE_STEAL].vColor = CLIENT_RGB(119.f, 225.f, 100.f);
			m_Particles[PARTICLE_STEAL].iNumParticles = 5;
			if (nullptr == CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_PARTICLE, &m_Particles[PARTICLE_STEAL]))
			{
				MSG_BOX(TEXT("fail effect"));
			}
			POINTLIGHTDESC* LIGHT;
			_pInstance->Light_On(g_eCurLevel, CLight_Manager::DYNAMICPOINTLIHGT, m_iClawLight);
			LIGHT = _pInstance->Get_PointLightDesc(g_eCurLevel, CLight_Manager::DYNAMICPOINTLIHGT, m_iClawLight);

			XMStoreFloat4(&(LIGHT->vPosition), XMVectorSetW(XMLoadFloat3(&m_vCurClaw), 1.f));
			LIGHT->fRange = 2.f;
		}
		else if (m_fPlayTime > 5.f)
		{
			AUTOINSTANCE(CGameInstance, _pInstance);
			if (!m_bEffect)
			{
				CCrossTrail::CROSS_DESC _tInfo;
				_tInfo.vRGBA = CLIENT_RGB(255.f, 255.f, 0.f);
				m_pClawTrail[NAIL_ONE] = static_cast<CCrossTrail*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_CROSSTRAIL, &(_tInfo)));
				Safe_AddRef(m_pClawTrail[NAIL_ONE]);
				_tInfo.vRGBA = CLIENT_RGB(119.f, 255.f, 130.f);
				m_pClawTrail[NAIL_TWO] = static_cast<CCrossTrail*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_CROSSTRAIL, &(_tInfo)));
				Safe_AddRef(m_pClawTrail[NAIL_TWO]);
				_tInfo.vRGBA = CLIENT_RGB(66.f, 236.f, 62.f);
				m_pClawTrail[NAIL_THREE] = static_cast<CCrossTrail*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_CROSSTRAIL, &(_tInfo)));
				Safe_AddRef(m_pClawTrail[NAIL_THREE]);
				_tInfo.vRGBA = CLIENT_RGB(119.f, 255.f, 10.f);
				m_pClawTrail[NAIL_FOUR] = static_cast<CCrossTrail*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_CROSSTRAIL, &(_tInfo)));
				Safe_AddRef(m_pClawTrail[NAIL_FOUR]);

				
				m_bEffect = true;
			}
			m_bCollision[COLLIDERTYPE_BODY] = false;
			m_bCollision[COLLIDERTYPE_CLAW] = true;
			Update_Nail();
			POINTLIGHTDESC* LIGHT;
			_pInstance->Light_On(g_eCurLevel, CLight_Manager::DYNAMICPOINTLIHGT, m_iClawLight);
			LIGHT = _pInstance->Get_PointLightDesc(g_eCurLevel, CLight_Manager::DYNAMICPOINTLIHGT, m_iClawLight);
			
			XMStoreFloat4(&(LIGHT->vPosition), XMVectorSetW(XMLoadFloat3( &m_vCurClaw),1.f));
			LIGHT->fRange = 2.f;
		}
		break;
	case Corvus_PW_Halberds:
		if (m_vecLimitTime[Corvus_PW_Halberds][6] < m_fPlayTime)
		{
			if (m_pSkillParts[SKILL_LENCE][0]->Trail_GetOn())
				m_pSkillParts[SKILL_LENCE][0]->TrailOff();
		}
		else if (m_vecLimitTime[Corvus_PW_Halberds][5] < m_fPlayTime)
		{
			if (!m_pSkillParts[SKILL_LENCE][0]->Trail_GetOn())
				m_pSkillParts[SKILL_LENCE][0]->TrailOn();
		}
		else if (m_vecLimitTime[Corvus_PW_Halberds][4] < m_fPlayTime)
		{
			if (m_pSkillParts[SKILL_LENCE][0]->Trail_GetOn())
				m_pSkillParts[SKILL_LENCE][0]->TrailOff();
			m_pSkillParts[SKILL_LENCE][0]->Set_CollisionOn(false);
			m_pStatusCom->Set_Attack(30.f);
		}
		else if (m_vecLimitTime[Corvus_PW_Halberds][3] < m_fPlayTime)
		{
			for (auto& _pEffect : m_vecWind)
				_pEffect->Setting(m_pTransformCom->Get_State(CTransform::STATE_POSITION), true);
		}
		else if (m_vecLimitTime[Corvus_PW_Halberds][2] < m_fPlayTime)
		{
			m_pSkillParts[SKILL_LENCE][0]->Set_CollisionOn(true);
			m_pStatusCom->Set_Attack(m_fSkillAtts[SKILL_LENCE] * m_fUseStealSkill);
			for (auto& _pEffect : m_vecWind)
				_pEffect->Setting(m_pTransformCom->Get_State(CTransform::STATE_POSITION), false);
		}
		else if (m_vecLimitTime[Corvus_PW_Halberds][1] < m_fPlayTime)
		{
			if (!m_bEffect)
			{
				CWind::WIND_DESC _tWind;
				CWind*	pEffect = nullptr;
				_tWind.fAngle = 20.f;
				XMStoreFloat4(&_tWind.TargetPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				pEffect = static_cast<CWind*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_WIND, &_tWind));
				m_vecWind.push_back(pEffect);
				Safe_AddRef(pEffect);

				_tWind.fAngle = 120.f;
				pEffect = static_cast<CWind*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_WIND, &_tWind));
				m_vecWind.push_back(pEffect);
				Safe_AddRef(pEffect);

				_tWind.fAngle = 240.f;
				pEffect = static_cast<CWind*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_WIND, &_tWind));
				m_vecWind.push_back(pEffect);
				Safe_AddRef(pEffect);

				m_bEffect = true;
			}
			for (auto& _pEffect : m_vecWind)
				_pEffect->Setting(m_pTransformCom->Get_State(CTransform::STATE_POSITION), false);
		}
		else if (m_vecLimitTime[Corvus_PW_Halberds][0] < m_fPlayTime)
		{
			if (!m_pSkillParts[SKILL_LENCE][0]->Trail_GetOn())
				m_pSkillParts[SKILL_LENCE][0]->TrailOn();
			
			
		}
		break;
	case Corvus_PW_Scythe:
		if (m_vecLimitTime[Corvus_PW_Scythe][1] < m_fPlayTime)
		{
			if (m_pSkillParts[SKILL_SCYTHE][0]->Trail_GetOn())
				m_pSkillParts[SKILL_SCYTHE][0]->TrailOff();
			m_pSkillParts[SKILL_SCYTHE][0]->Set_CollisionOn(false);
			m_pStatusCom->Set_Attack(30.f);
			_bool _bFire(false);
			m_pSkillParts[SKILL_SCYTHE][0]->Clear(&_bFire);
		}
		else if (m_vecLimitTime[Corvus_PW_Scythe][0] < m_fPlayTime)
		{
			if (!m_pSkillParts[SKILL_SCYTHE][0]->Trail_GetOn())
				m_pSkillParts[SKILL_SCYTHE][0]->TrailOn();
		}
		break;
	case Corvus_VSMagician_Execution:
		if (m_vecLimitTime[Corvus_VSMagician_Execution][1] < m_fPlayTime)
		{
			
		}
		else if (m_vecLimitTime[Corvus_VSMagician_Execution][0] < m_fPlayTime)
		{
			static_cast<CSaber*>(m_pBaseParts[BASE_SABER])->Blood();
		}
		break;
	case Corvus_VSBossBat_Execution01:
		if (m_vecLimitTime[Corvus_VSBossBat_Execution01][1] < m_fPlayTime)
		{

		}
		else if (m_vecLimitTime[Corvus_VSBossBat_Execution01][0] < m_fPlayTime)
		{
			static_cast<CSaber*>(m_pBaseParts[BASE_SABER])->Blood();
		}
		break;
	case Corvus_VSLV2Villager_M_Execution:
		if (m_vecLimitTime[Corvus_VSLV2Villager_M_Execution][2] < m_fPlayTime)
		{

		}
		else if (m_vecLimitTime[Corvus_VSLV2Villager_M_Execution][1] < m_fPlayTime)
		{/*
			m_vPreClaw = m_vCurClaw;
			XMStoreFloat3(&m_vCurClaw, (m_pHands[HAND_RIGHT]->Get_CombinedTransformation()*XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())*m_pTransformCom->Get_WorldMatrix()).r[3]);
			m_Particles[PARTICLE_BLOOD].Center = m_vCurClaw; */
			//static_cast<CSaber*>(m_pBaseParts[BASE_SABER])->Blood();
			/*XMStoreFloat3(&m_Particles[PARTICLE_BLOOD].vStart_Dir, XMVector3Normalize(XMLoadFloat3(&m_vPreClaw) - XMLoadFloat3(&m_vCurClaw)));
			if (nullptr == CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_PARTICLE, &m_Particles[PARTICLE_BLOOD]))
			{
				MSG_BOX(TEXT("fail effect"));
			}*/
		}
		else if (m_vecLimitTime[Corvus_VSLV2Villager_M_Execution][0] < m_fPlayTime)
		{
			//m_vPreClaw = m_vCurClaw;
			//XMStoreFloat3(&m_vCurClaw, (m_pHands[HAND_RIGHT]->Get_CombinedTransformation()*XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())*m_pTransformCom->Get_WorldMatrix()).r[3]);
		}
		break;		
	}

}

_bool CPlayer::CheckLimit_Att(STATE _eAtt)
{


	if (m_fPlayTime > m_vecLimitTime[_eAtt][ATTACKLIMIT_CHANGE])
	{
		if (m_eReserveState != STATE_END)
		{
			Change_Anim();
			return true;
		}
	}

	if (m_fPlayTime < m_vecLimitTime[_eAtt][ATTACKLIMIT_MOVE])
	{
		if (m_pTarget)
		{
			_float4x4 _CamMatix;
			XMStoreFloat4x4(&_CamMatix, XMMatrixInverse(nullptr, CGameInstance::Get_Instance()->Get_TransformMatrix(CPipeLine::D3DTS_VIEW)));

			_vector	_vCamLook, _vCamRight, _vLook;
			_float _fRatio = 0.8f;
			_vCamLook = XMLoadFloat4((_float4*)&_CamMatix.m[2][0]);
			_vCamRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), _vCamLook));
			_vCamLook = XMVector3Normalize(XMVector3Cross(_vCamRight, XMVectorSet(0.f, 1.f, 0.f, 0.f)));

			_vLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
			m_pTransformCom->Turn(_vLook, _vCamLook, _fRatio);
		}
	}

	if (m_fPlayTime > m_vecLimitTime[_eAtt][ATTACKLIMIT_TRAILEND])
	{
		if (m_pBaseParts[BASE_SABER]->Trail_GetOn() == true && m_eReserveState == STATE_END)
		{
			m_pBaseParts[BASE_SABER]->TrailOff();
		}
	}
	else if (m_fPlayTime > m_vecLimitTime[_eAtt][ATTACKLIMIT_TRAILON])
	{
		//트레일 온
		if (m_pBaseParts[BASE_SABER]->Trail_GetOn() == false)
		{
			m_pBaseParts[BASE_SABER]->TrailOn();
		}
	}

	if (m_fPlayTime > m_vecLimitTime[_eAtt][ATTACKLIMIT_COLLIDEREND])
	{
		m_pBaseParts[BASE_SABER]->Set_CollisionOn(false);
	}
	else if (m_fPlayTime > m_vecLimitTime[_eAtt][ATTACKLIMIT_COLLIDERON])
	{
		m_pBaseParts[BASE_SABER]->Set_CollisionOn(true);
	}
	return false;
}

void CPlayer::Change_Anim()
{
	m_eCurState = m_eReserveState;
	m_eReserveState = STATE_END;
	Set_Anim(m_eCurState);
	XMStoreFloat4(&m_AnimPos, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	m_PreAnimPos = m_AnimPos;
}

void CPlayer::AfterAnim(_float fTimeDelta)
{
	//AUTOINSTANCE(CCameraMgr, _pCamera);
	switch (m_eCurState)
	{
	case Client::CPlayer::STATE_ATT1:
		//CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::GAMEOBJ_TYPE::TYPE_PLAYER_BODY, m_pColliderCom[COLLIDERTYPE_BODY], this);
		m_bCollision[COLLIDERTYPE_BODY] = true;
		break;
	case Client::CPlayer::STATE_ATT2:
		//CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::GAMEOBJ_TYPE::TYPE_PLAYER_BODY, m_pColliderCom[COLLIDERTYPE_BODY], this);
		m_bCollision[COLLIDERTYPE_BODY] = true;
		break;
	case Client::CPlayer::STATE_ATT3:
		m_bCollision[COLLIDERTYPE_BODY] = true;
		break;
	case Client::CPlayer::STATE_ATT4:
		m_bCollision[COLLIDERTYPE_BODY] = true;
		break;
	case Client::CPlayer::STATE_ATT5:
		m_bCollision[COLLIDERTYPE_BODY] = true;
		break;
	case Client::CPlayer::STATE_RUN_B:
		break;
	case Client::CPlayer::STATE_RUN_F:
		if (g_eCurLevel == LEVEL_STAGE_LOBBY)
			Gate();
		CutScene();
		m_bCollision[COLLIDERTYPE_BODY] = true;
		break;
	case Client::CPlayer::STATE_RUN_L:
		break;
	case Client::CPlayer::STATE_RUN_R:
		break;
	case Client::CPlayer::STATE_APPROACH:
		m_eWeapon = WEAPON_NONE;
		break;
	case Client::CPlayer::STATE_WALK:
		break;
	case Client::CPlayer::STATE_IDLE:
		m_bCollision[COLLIDERTYPE_BODY] = true;
		break;
	case Client::CPlayer::STATE_AVOIDATTACK:
		if (m_pBaseParts[BASE_SABER]->Trail_GetOn())
			m_pBaseParts[BASE_SABER]->TrailOff();
		m_bCollision[COLLIDERTYPE_BODY] = false;
		break;
	case Client::CPlayer::STATE_JUMPAVOID:
		m_bMotionPlay = true;
		break;
	case Client::CPlayer::STATE_AVOIDBACK:
		m_bMotionPlay = true;
		if (m_pBaseParts[BASE_SABER]->Trail_GetOn())
			m_pBaseParts[BASE_SABER]->TrailOff();
		m_bCollision[COLLIDERTYPE_BODY] = false;
		break;
	case Client::CPlayer::Corvus_PW_Axe:
		break;
	case Client::CPlayer::Tackle_F:
		break;
	case Client::CPlayer::ParryR:
		m_bCollision[COLLIDERTYPE_BODY] = true;

		break;
	case Client::CPlayer::ParryL:
		m_bCollision[COLLIDERTYPE_BODY] = true;

		break;
	case Client::CPlayer::DualKnife:
		break;
	case Client::CPlayer::GreatSword:
		break;
	case Client::CPlayer::PW_Halberds:
		break;
	case Client::CPlayer::PW_Hammer_A:
		break;
	case Client::CPlayer::PW_TwinSwords_1:
		break;
	case Client::CPlayer::PW_TwinSwords_2:
		break;
	case Client::CPlayer::PW_VargSword_A:
		break;
	case Client::CPlayer::PW_Bow_Start:
		break;
	case Client::CPlayer::PW_Bow_End:
		break;
	case Client::CPlayer::PW_Bow_B:
		break;
	case Client::CPlayer::PW_Bloodwhip:
		break;
	case Client::CPlayer::PW_CaneSword_SP01:
		break;
	case Client::CPlayer::PW_CaneSword_SP02:
		break;
	case Client::CPlayer::Healing_Little:
		break;
	case Client::CPlayer::Healing2_Blend:
		break;
	case Client::CPlayer::Raven_ClawCommonV2_ChargeStart:
		break;
	case Client::CPlayer::Raven_ClawLong_ChargeFull:
		break;
	case Client::CPlayer::Raven_ClawNear:
		m_eWeapon = WEAPON_NONE;
		break;
	case Client::CPlayer::Strong_Jump2:
	{
		
	}

		break;
	case Client::CPlayer::RavenAttack_Start:
		break;
	case Client::CPlayer::RavenAttack_End:
		break;
	case Client::CPlayer::SD_ParryToMob:
		break;
	case Client::CPlayer::SD_HurtIdle:
		break;
	case Client::CPlayer::SD_StrongHurt_Start:
		break;
	case Client::CPlayer::SD_StrongHurt_End:
		break;
	case Client::CPlayer::SD_Dead:
		break;
	case Client::CPlayer::Corvus_VSLV1Villager_M_Execution:
		if (m_bOneChange == false)
		{
			Change_Hand();
			m_bOneChange = true;
		

		}
		break;
	case Client::CPlayer::Corvus_VSLV2Villager_M_Execution:
		if (m_bOneChange == false)
		{
			Change_Hand();
			m_bOneChange = true;
			//m_eWeapon = WEAPON_NONE;
		}

		break;
	case Client::CPlayer::Corvus_VSMagician_Execution:
	{
		AUTOINSTANCE(CGameInstance, _Instance);
		_Instance->Set_TimeSpeed(TEXT("Timer_Main"), 0.75f);
	}
		
		Cancle();
		break;
	case Client::CPlayer::Corvus_VSBossBat_Execution01:
		Cancle();
		break;
	case Client::CPlayer::Corvus_SD_Fall_Loop:
		m_eWeapon = WEAPON_NONE;
		m_pTransformCom->Go_Up(-fTimeDelta * m_fAddSpeed * 0.7f);
		m_fAddSpeed *= 1.1f;
		if (g_eCurLevel == LEVEL_STAGE_02)
		{
			AUTOINSTANCE(CCameraMgr, _pCamera);
			_pCamera->Change_Camera(CCameraMgr::CAMERA_CUTSCENE_ENTER);

			_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
			if (vPos.m128_f32[1] < 1.5f)
			{
				AUTOINSTANCE(CGameInstance, _pGameInstance);
				m_eCurState = Corvus_SD_Fall_End;
				//_pGameInstance->Set_TimeSpeed(TEXT("Timer_Main"), 0.1f);
			}
		}
		break;
	case Client::CPlayer::Corvus_SD_Fall_End:
		_vector vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		if (vPos.m128_f32[1] > 0.f)
		{
			m_pTransformCom->Go_Up(-fTimeDelta * m_fAddSpeed * 0.7f);
			m_fAddSpeed *= 1.1f;
		}
		else
		{
			vPos.m128_f32[1] = 0.f;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);
		}
		

		break;

	case Corvus_VS_Puppet_Execution_Start:
	{
		//90도? ㄴㄴ 퍼펫 바라보자 퍼펫쪽으로 향하면서 뛰면 되니까 열심히 뛰어보자
		
		/*CTransform* _Trans =
			static_cast<CTransform*>(m_MonsterLayer->Get_ObjFromLayer(0)->Get_ComponentPtr(TEXT("Com_Transform")));

		_vector _vLookStart = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
		_vector _vLookEnd = XMVector3Normalize((_Trans->Get_State(CTransform::STATE_POSITION)) - (m_pTransformCom->Get_State(CTransform::STATE_POSITION)));

		if (XMVector3Length(_vLookStart - _vLookEnd).m128_f32[0] < 0.03f)
		{
			m_pTransformCom->LookAt_ForLandObject(_Trans->Get_State(CTransform::STATE_POSITION));
		}
		else
		{
			m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_LOOK), _vLookEnd, 0.99f);
		}*/

		AUTOINSTANCE(CGameInstance, _pGameInstance);
		POINTLIGHTDESC*			LightDesc = nullptr;
		LightDesc = (_pGameInstance->Get_PointLightDesc(LEVEL_STAGE_LAST, CLight_Manager::STATICPOINTLIHGT, m_iLightIndex));
		_vector _vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		_vPos.m128_f32[1] += 0.5f;
		XMStoreFloat4(&(LightDesc->vPosition), _vPos);
	}
		break;

	case Corvus_VS_Puppet_Execution_Attack:
	{
		AUTOINSTANCE(CGameInstance, _pGameInstance);
		POINTLIGHTDESC*			LightDesc = nullptr;
		LightDesc = (_pGameInstance->Get_PointLightDesc(LEVEL_STAGE_LAST, CLight_Manager::STATICPOINTLIHGT, m_iLightIndex));
		_vector _vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		_vPos.m128_f32[1] += 0.5f;
		XMStoreFloat4(&(LightDesc->vPosition), _vPos);
	}
		break;
	case Corvus_VS_Puppet_Execution_End:
	{
		AUTOINSTANCE(CGameInstance, _pGameInstance);
		POINTLIGHTDESC*			LightDesc = nullptr;
		LightDesc = (_pGameInstance->Get_PointLightDesc(LEVEL_STAGE_LAST, CLight_Manager::STATICPOINTLIHGT, m_iLightIndex));
		_vector _vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		_vPos.m128_f32[1] += 0.5f;
		XMStoreFloat4(&(LightDesc->vPosition), _vPos);
	}
		break;
	case Client::CPlayer::Corvus_Raven_ClawLong_ChargeFull:
	{
		//거리가 어느정도 되었을때 이걸로 바뀐다.
		// 끝까지 안닿으면 뭐 쩔수없궁 히히
		if (m_pTarget == nullptr)
		{
			m_eCurState = STATE_IDLE;
		}
		{
			_vector _vTargetPos = static_cast<CTransform*>(m_pTarget->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION);
			_vector _vPlayerPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
			if (XMVector3Length(_vTargetPos - _vPlayerPos).m128_f32[0] < 1.f)
			{
				m_eReserveState = Corvus_Raven_ClawLong_PlunderAttack2TTT;
				m_bCollision[COLLIDERTYPE_CLAW] = true;
				m_bCollision[COLLIDERTYPE_BODY] = false;
			}
		}
		
	}
		
		break;
	case Client::CPlayer::Corvus_Raven_ClawLong_PlunderAttack2TTT:	
		//Update_Nail();
		break;

		
	}
	Get_AnimMat();
}

void CPlayer::Set_Anim(STATE _eState)
{
	m_eCurState = _eState;
	XMStoreFloat4(&m_AnimPos, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	m_PreAnimPos = m_AnimPos;
	m_pModelCom->Set_AnimationIndex(m_eCurState);
	m_fPlayTime = 0.f;

	m_pBaseParts[BASE_SABER]->Set_CollisionOn(false);
	m_bMotionPlay = false;
}

void CPlayer::CheckAnim()
{
	if (m_ePreState != m_eCurState)
	{
		Set_Anim(m_eCurState);
		m_ePreState = m_eCurState;
	}
}

void CPlayer::Get_AnimMat()
{
	if (m_bAnimStop)
		return;
	_fmatrix _World = m_pTransformCom->Get_WorldMatrix();
	_vector _vPos;
	_vPos = XMVector3TransformCoord(XMLoadFloat4(&m_AnimPos), _World);

	_bool		isMove = true;

	_vector		vNormal = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	if (nullptr != m_pNavigationCom)
	{
		isMove = m_pNavigationCom->isMove(_vPos, nullptr/*&vNormal*/);
		if (true == isMove)
		{
			_vPos.m128_f32[1] = m_pNavigationCom->Get_PosY(_vPos);
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);
		}
	}
	else
	{
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);
	}
		

	

}

void CPlayer::Execution()
{
	AUTOINSTANCE(CGameInstance, _pInstance);
	list<CGameObject*> Monsters = *m_MonsterLayer->Get_ListFromLayer();
	CGameObject*		_Target = nullptr;
	_vector _vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float _fClosedDis(1.5f);
	for (auto iter : Monsters)
	{
		_float _fDis = fabs(XMVectorGetX(XMVector3Length(static_cast<CTransform*>(iter->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION)	- _vPos)));
		if (_fDis < 1.5f)
		{
			if (_fClosedDis > _fDis)
			{
				_Target = iter;
				_fClosedDis = _fDis;
			}
		}
	}
	if (_Target != nullptr)
	{
		CMonster* _pMonster = static_cast<CMonster*>(_Target);
		CAnimModel* _pModel = static_cast<CAnimModel*>(_Target->Get_ComponentPtr(TEXT("Com_Model")));
		switch (_pMonster->Get_Type())
		{
		case CMonster::MONSTER_EXTRA01:
			if (false == static_cast<CExtra01*>(_pMonster)->Get_Battle() || _pMonster->Get_MonsterState() == CMonster::ATTACK_GROGGY)
			{
				AUTOINSTANCE(CCameraMgr, _pCamera);

				static_cast<CExtra01*>(_pMonster)->Set_AnimState(CExtra01::LV1Villager_M_VSTakeExecution);
				m_eCurState = Corvus_VSLV1Villager_M_Execution;
				m_pModelCom->DirectAnim(Corvus_VSLV1Villager_M_Execution);
				_pModel->DirectAnim(CExtra01::LV1Villager_M_VSTakeExecution);
				CTransform* _TargetTrans = static_cast<CTransform*>(_pMonster->Get_ComponentPtr(TEXT("Com_Transform")));

				m_pTransformCom->LookAt_ForLandObject(_TargetTrans->Get_State(CTransform::STATE_POSITION));
				_vector vTargetPos = (m_pModelCom->Get_HierarchyNode("AnimTargetPoint")->Get_CombinedTransformation()
					*XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())*m_pTransformCom->Get_WorldMatrix()).r[3];
				_TargetTrans->Set_State(CTransform::STATE_POSITION, vTargetPos);
				_TargetTrans->LookAt_ForLandObject(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				_pInstance->Set_TimeSpeed(TEXT("Timer_Main"), 0.5f);
				_pCamera->Get_Cam(CCameraMgr::CAMERA_PLAYER)->ZoomIn(40.f, 80.f);
				m_bCollision[COLLIDERTYPE_BODY] = false;
			}
			break;
		case CMonster::MONSTER_EXTRA02:
			if (false == static_cast<CExtra02*>(_pMonster)->Get_Battle() || _pMonster->Get_MonsterState() == CMonster::ATTACK_GROGGY)
			{
				AUTOINSTANCE(CCameraMgr, _pCamera);

				static_cast<CExtra02*>(_pMonster)->Set_AnimState(CExtra02::LV2Villager01_M_VS_TakeExecution_01);
				m_eCurState = Corvus_VSLV2Villager_M_Execution;
				m_pModelCom->DirectAnim(Corvus_VSLV2Villager_M_Execution);
				_pModel->DirectAnim(CExtra02::LV2Villager01_M_VS_TakeExecution_01);
				CTransform* _TargetTrans = static_cast<CTransform*>(_pMonster->Get_ComponentPtr(TEXT("Com_Transform")));

				m_pTransformCom->LookAt_ForLandObject(_TargetTrans->Get_State(CTransform::STATE_POSITION));
				_vector vTargetPos = (m_pModelCom->Get_HierarchyNode("AnimTargetPoint")->Get_CombinedTransformation()
					*XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())*m_pTransformCom->Get_WorldMatrix()).r[3];
				_TargetTrans->Set_State(CTransform::STATE_POSITION, vTargetPos);
				_TargetTrans->LookAt_ForLandObject(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				_pInstance->Set_TimeSpeed(TEXT("Timer_Main"), 0.3f);
				_pCamera->Get_Cam(CCameraMgr::CAMERA_PLAYER)->ZoomIn(40.f, 80.f);
				m_bCollision[COLLIDERTYPE_BODY] = false;
			}
			break;
		case CMonster::MONSTER_EXTRA01_LAST:
			if (false == static_cast<CExtra01_Last*>(_pMonster)->Get_Battle() || _pMonster->Get_MonsterState() == CMonster::ATTACK_GROGGY)
			{
				AUTOINSTANCE(CCameraMgr, _pCamera);

				static_cast<CExtra01_Last*>(_pMonster)->Set_AnimState(CExtra01_Last::LV1Villager_M_VSTakeExecution);
				m_eCurState = Corvus_VSLV1Villager_M_Execution;
				m_pModelCom->DirectAnim(Corvus_VSLV1Villager_M_Execution);
				_pModel->DirectAnim(CExtra01_Last::LV1Villager_M_VSTakeExecution);
				CTransform* _TargetTrans = static_cast<CTransform*>(_pMonster->Get_ComponentPtr(TEXT("Com_Transform")));

				m_pTransformCom->LookAt_ForLandObject(_TargetTrans->Get_State(CTransform::STATE_POSITION));
				_vector vTargetPos = (m_pModelCom->Get_HierarchyNode("AnimTargetPoint")->Get_CombinedTransformation()
					*XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())*m_pTransformCom->Get_WorldMatrix()).r[3];
				_TargetTrans->Set_State(CTransform::STATE_POSITION, vTargetPos);
				_TargetTrans->LookAt_ForLandObject(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				_pInstance->Set_TimeSpeed(TEXT("Timer_Main"), 0.5f);
				_pCamera->Get_Cam(CCameraMgr::CAMERA_PLAYER)->ZoomIn(40.f, 80.f);
				m_bCollision[COLLIDERTYPE_BODY] = false;
			}
			break;
		case CMonster::MONSTER_EXTRA02_LAST:
			if (false == static_cast<CExtra02_Last*>(_pMonster)->Get_Battle() || _pMonster->Get_MonsterState() == CMonster::ATTACK_GROGGY)
			{
				AUTOINSTANCE(CCameraMgr, _pCamera);

				static_cast<CExtra02_Last*>(_pMonster)->Set_AnimState(CExtra02_Last::LV2Villager01_M_VS_TakeExecution_01);
				m_eCurState = Corvus_VSLV2Villager_M_Execution;
				m_pModelCom->DirectAnim(Corvus_VSLV2Villager_M_Execution);
				_pModel->DirectAnim(CExtra02_Last::LV2Villager01_M_VS_TakeExecution_01);
				CTransform* _TargetTrans = static_cast<CTransform*>(_pMonster->Get_ComponentPtr(TEXT("Com_Transform")));

				m_pTransformCom->LookAt_ForLandObject(_TargetTrans->Get_State(CTransform::STATE_POSITION));
				_vector vTargetPos = (m_pModelCom->Get_HierarchyNode("AnimTargetPoint")->Get_CombinedTransformation()
					*XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())*m_pTransformCom->Get_WorldMatrix()).r[3];
				_TargetTrans->Set_State(CTransform::STATE_POSITION, vTargetPos);
				_TargetTrans->LookAt_ForLandObject(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
				_pInstance->Set_TimeSpeed(TEXT("Timer_Main"), 0.3f);
				_pCamera->Get_Cam(CCameraMgr::CAMERA_PLAYER)->ZoomIn(40.f, 80.f);
				m_bCollision[COLLIDERTYPE_BODY] = false;
			}
			break;
		}
	}
}

void CPlayer::Cancle()
{
	//무기들 전부 끄고, 근데 스킬 중엔 충돌 자체가 없을거고, 패링이 없을텐데...

	m_bCollision[COLLIDERTYPE_BODY] = true;
	m_bCollision[COLLIDERTYPE_CLAW] = false;
	m_bCollision[COLLIDERTYPE_PARRY] = false;

	for (auto& _pParts : m_pBaseParts)
	{
		if (_pParts->Trail_GetOn())
			_pParts->TrailOff();

		_pParts->Set_CollisionOn(false);
	}
}

void CPlayer::Update_Collider()
{
	m_pColliderCom[COLLIDERTYPE_PUSH]->Update(m_pTransformCom->Get_WorldMatrix());
	CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_PLAYER_PUSH, m_pColliderCom[COLLIDERTYPE_PUSH], this);
	if (m_bCollision[COLLIDERTYPE_CLAW])
	{
		m_vPreClaw = m_vCurClaw;
		XMStoreFloat3(&m_vCurClaw, (m_pHands[HAND_RIGHT]->Get_CombinedTransformation()*XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())*m_pTransformCom->Get_WorldMatrix()).r[3]);
		m_pColliderCom[COLLIDERTYPE_CLAW]->Update(m_pHands[HAND_RIGHT]->Get_CombinedTransformation()*XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())*m_pTransformCom->Get_WorldMatrix());
		CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_PLAYER_WEAPON, m_pColliderCom[COLLIDERTYPE_CLAW], this);
	}
	if (m_bCollision[COLLIDERTYPE_BODY])
	{
		m_pColliderCom[COLLIDERTYPE_BODY]->Update(m_pTransformCom->Get_WorldMatrix());
		CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_PLAYER_BODY, m_pColliderCom[COLLIDERTYPE_BODY], this);
	}
	if (m_bCollision[COLLIDERTYPE_PARRY])
	{
		m_pColliderCom[COLLIDERTYPE_PARRY]->Update(m_pTransformCom->Get_WorldMatrix());
		CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_PLAYER_PARRY, m_pColliderCom[COLLIDERTYPE_PARRY], this);
	}
}

_bool CPlayer::Collision(_float fTimeDelta)
{
	CGameObject* _pTarget = m_pColliderCom[COLLIDERTYPE_CLAW]->Get_Target();
	if (_pTarget && m_eCurState == Corvus_Raven_ClawLong_PlunderAttack2TTT)
	{
		if (/*_pTarget == m_pTarget && */m_eStealSkill == STATE_END)
		{

			AUTOINSTANCE(CGameInstance, _pGame);

			_int RandInt = _pGame->Rand_Int(0, SKILL_END - 1);

			switch (RandInt)
			{
			case SKILL_AXE:
				m_eStealSkill = Corvus_PW_Axe;
				break;
			case SKILL_DUAL:
				m_eStealSkill = DualKnife;
				break;
			case SKILL_BOW:
				m_eStealSkill = PW_Bow_Start;
				break;
			case SKILL_SCYTHE:
				m_eStealSkill = Corvus_PW_Scythe;
				break;
			case SKILL_LENCE:
				m_eStealSkill = Corvus_PW_Halberds;
				break;
			}

			static_cast<CSkillSlot*>(CUI_Mgr::Get_Instance()->Get_UI(TEXT("PLAYER_SLOT")))->Set_Skill(RandInt);
		}
	}

	_pTarget = m_pColliderCom[COLLIDERTYPE_PARRY]->Get_Target();
	if (_pTarget && _pTarget->Get_ObjType() != TYPE_BULLET)
	{
		//MSG_BOX(TEXT("Parry"));
		static_cast<CMonster*>(_pTarget)->Set_MonsterState(CMonster::ATTACK_STUN);
		m_iHitCount = 0;
	}
	else
	{
		_pTarget = (m_pColliderCom[COLLIDERTYPE_BODY])->Get_Target();
		if (_pTarget)
		{
			if (_pTarget->Get_ObjType() == TYPE_MONSTER)
			{
				static_cast<CMonster*>(_pTarget)->Set_Coll_Target(true);
			}

			//패링 안되고 몸 충돌되었을때
			
			if (m_iHitCount > 2)
			{
				m_eCurState = SD_StrongHurt_Start;
			}
			else if (m_eCurState == SD_HurtIdle)
			{
				if (m_bAgainAnim == false)
				{
					m_bAgainAnim = true;
					Set_Anim(m_eCurState);
				}
				++m_iHitCount;
			}
			else
			{
				m_eCurState = SD_HurtIdle;
				++m_iHitCount;
			}

			for (auto& _Part : m_pBaseParts)
			{
				if (_Part->Trail_GetOn())
				{
					_Part->TrailOff();
				}
				
				_Part->Set_CollisionOn(false);
			}
			m_pTransformCom->LookAt_ForLandObject(
				static_cast<CTransform*>(_pTarget->Get_ComponentPtr(TEXT("Com_Transform")))
				->Get_State(CTransform::STATE_POSITION));
			m_bCollision[COLLIDERTYPE_CLAW] = false;
			m_bCollision[COLLIDERTYPE_PARRY] = false;

			m_eWeapon = WEAPON_BASE;
			m_pStatusCom->Damage(static_cast<CStatus*>(_pTarget->Get_ComponentPtr(TEXT("Com_Status")))->Get_Attack());
			m_bCollision[COLLIDERTYPE_BODY] = false;
			return true;
		}
		else if (_pTarget = (m_pColliderCom[COLLIDERTYPE_CLAW])->Get_Target())
		{
			_vector _vPos = static_cast<CTransform*>(_pTarget->Get_ComponentPtr(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION);
			_vector _vPlayerPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
			_vPos.m128_f32[1] += 1.7f;
			_vPlayerPos.m128_f32[1] += 1.7f;
			XMStoreFloat3(&(m_Particles[PARTICLE_CLAW].vStart_Dir), _vPlayerPos - _vPos/*XMLoadFloat3(&m_vCurClaw)*/);
			m_Particles[PARTICLE_CLAW].Center = m_vCurClaw;
			m_Particles[PARTICLE_CLAW].vColor =CLIENT_RGB(119.f, 245.f, 110.f);

			if (nullptr == CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_PARTICLE, &m_Particles[PARTICLE_CLAW]))
			{
				MSG_BOX(TEXT("fail effect"));
			}
			m_Particles[PARTICLE_CLAW].vColor = CLIENT_RGB(255.f, 127.f, 0.f);
			if (nullptr == CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_PARTICLE, &m_Particles[PARTICLE_CLAW]))
			{
				MSG_BOX(TEXT("fail effect"));
			}
		}
		

	}
	return false;
}

void CPlayer::Update_Nail()
{
	_float3 _vPos;
	_matrix _NailMatrix;
	for (_uint i = 0; i < NAIL_END; ++i)
	{
		if (m_pNails[i] == nullptr)
			continue;
		_NailMatrix = m_pNails[i]->Get_CombinedTransformation()
			* XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())
			* m_pTransformCom->Get_WorldMatrix();
		XMStoreFloat3(&_vPos, _NailMatrix.r[3]);
		m_pClawTrail[i]->Add_Point(_vPos);
	}

	// 파티클? 사방으로 다 퍼지게끔? 일단 손에 점광원 하나 넣고, 그 순간에 피 쫘악 한번 해주고
	// 
	//m_Particles[PARTICLE_STEAL].Center = _vPos;

}

HRESULT CPlayer::Ready_ParticleDesc()
{
	CEffect_Particle::OPTION _tOption;
	//_tOption.Center = _float3(45.f, 2.f, 45.f);
	_vector	_vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	_float4x4	_vWorld = m_pTransformCom->Get_WorldFloat4x4();;
	_tOption.Center = _float3(_vPos.m128_f32[0], _vPos.m128_f32[1] + 1.f, _vPos.m128_f32[2]);
	_tOption.eType = CEffect_Particle::PARTICLETYPE::TYPE_STRIGHT;
	_tOption.fAccSpeed = 0.99f;
	_tOption.fSpeed = { 5.f, 7.f };
	_tOption.fGravity = 0.f;
	_tOption.fLifeTime = 0.f;
	_tOption.fRange = _float3(5.f, 5.f, 1.f);
	_tOption.iNumParticles = 15;
	_tOption.Size = _float2(0.2f, 0.2f);
	_tOption.Spread = CEffect_Particle::SPREAD::SPREAD_EDGE;
	_tOption.szMaskTag = TEXT("Prototype_Component_Texture_Mask_ClawEffect");
	_tOption.szTextureTag = TEXT("Prototype_Component_Texture_Diffuse_Blood");
	_tOption.vColor = CLIENT_RGB(119.f, 245.f, 110.f);
	//_tOption.vColor = CLIENT_RGB(82.f, 9.f, 4.f);
	//_tOption.bPlayerDir = true;
	_tOption.fSpead_Angle = _float3(0.f, 20.f, 20.f);
	_tOption.vStart_Dir = _float3(1.f, 0.f, 0.f);
	_tOption.eDiffuseType = CEffect_Particle::DIFFUSE_COLOR;
	_tOption.eDirType = CEffect_Particle::DIR_TYPE::DIR_ANGLE;
	_tOption.eStartType = CEffect_Particle::START_CENTER;
	_tOption.fMaxDistance = { 2.5f, 3.f };
	_tOption.bPlayerDir = false;
	_tOption.matPlayerAxix = _vWorld;

	m_Particles[PARTICLE_CLAW] = _tOption;

	_tOption.eType = CEffect_Particle::PARTICLETYPE::TYPE_STRIGHT;
	_tOption.fAccSpeed = 0.99f;
	_tOption.fSpeed = { 1.f, 3.f };
	_tOption.fGravity = 0.f;
	_tOption.fLifeTime = 0.f;
	_tOption.fRange = _float3(0.3f, 0.3f, 0.3f);
	_tOption.iNumParticles = 15;
	_tOption.Size = _float2(0.2f, 0.2f);
	_tOption.Spread = CEffect_Particle::SPREAD::SPREAD_EDGE;
	_tOption.szMaskTag = TEXT("Prototype_Component_Texture_Mask_ClawEffect");
	_tOption.szTextureTag = TEXT("Prototype_Component_Texture_Diffuse_Blood");
	_tOption.vColor = CLIENT_RGB(119.f, 245.f, 110.f);
	_tOption.eDiffuseType = CEffect_Particle::DIFFUSE_COLOR;
	_tOption.eDirType = CEffect_Particle::DIR_TYPE::DIR_NORMAL;
	_tOption.eStartType = CEffect_Particle::START_RANDOM;
	_tOption.fMaxDistance = { 2.5f, 3.f };
	_tOption.bPlayerDir = false;
	_tOption.eDissapear = CEffect_Particle::DISSAPEAR_ALPHA;

	m_Particles[PARTICLE_STEAL] = _tOption;


	_tOption.eType = CEffect_Particle::PARTICLETYPE::TYPE_STRIGHT;
	_tOption.fAccSpeed = 0.999f;
	_tOption.fSpeed = { 1.f, 3.f };
	_tOption.fGravity = 0.f;
	_tOption.fLifeTime = 0.f;
	_tOption.fRange = _float3(5.f, 5.f, 1.f);
	_tOption.iNumParticles = 5;
	_tOption.Size = _float2(0.15f, 0.15f);
	_tOption.Spread = CEffect_Particle::SPREAD::SPREAD_EDGE;
	_tOption.szMaskTag = TEXT("Prototype_Component_Texture_Mask_Blood_Exe");
	_tOption.szTextureTag = TEXT("Prototype_Component_Texture_Diffuse_Blood");
	//_tOption.vColor = CLIENT_RGB(119.f, 245.f, 200.f);
	//_tOption.vColor = CLIENT_RGB(255.f, 9.f, 4.f);
	_tOption.vColor = CLIENT_RGB(175.f, 17.f, 28.f);
	_tOption.bPlayerDir = false;
	_tOption.fSpead_Angle = _float3(0.f, 10.f, 10.f);
	XMStoreFloat3(&_tOption.vStart_Dir, XMVector3Normalize(XMLoadFloat3(& m_vCurClaw) - XMLoadFloat3(&m_vPreClaw)));
	_tOption.eDiffuseType = CEffect_Particle::DIFFUSE_COLOR;
	_tOption.eDirType = CEffect_Particle::DIR_TYPE::DIR_ANGLE;
	_tOption.eStartType = CEffect_Particle::START_CENTER;
	_tOption.fMaxDistance = { 0.8f, 1.2f };

	m_Particles[PARTICLE_BLOOD] = _tOption;

	
	return S_OK;
}

HRESULT CPlayer::Check_MotionTrail(_float fTimeDelta)
{
	m_CurMotionTrail += fTimeDelta;
	for (auto iter = m_listMotion.begin(); iter != m_listMotion.end();)
	{
		(*iter)->Tick(fTimeDelta);
		if ((*iter)->Get_Tick() > 0.5f)
		{
			(*iter)->Set_Tick(0.f);
			CDummy* _Motion = (*iter);
			iter = m_listMotion.erase(iter);
			m_listDeadMotion.push_back(_Motion);
		}
		else
			++iter;
	}
	if (!m_bMotionPlay)
	{
		return S_OK;
	}

	if (m_MaxMotionTrail < m_CurMotionTrail)
	{
		m_CurMotionTrail = 0.f;
		if (m_listDeadMotion.empty())
		{
			CMotionTrail* _Motion = CMotionTrail::Create(m_pDevice, m_pContext, m_pModelCom);
			if (_Motion == nullptr)
			{
				MSG_BOX(TEXT("creat Motion"));
				return E_FAIL;
			}
			_Motion->Set_CombinedMat(m_pModelCom->Get_HierarchyNodeVector(), m_pTransformCom->Get_WorldFloat4x4());
			CDummy*		_pDummy = CDummy::Creat(m_pDevice, m_pContext, _Motion, m_pTransformCom->Get_WorldFloat4x4());
			m_listMotion.push_back(_pDummy);
		}
		else
		{
			CDummy*		_pDummy = m_listDeadMotion.back();
			_pDummy->Set_CombinedMat(m_pModelCom->Get_HierarchyNodeVector(), m_pTransformCom->Get_WorldFloat4x4());
			m_listMotion.push_back(_pDummy);
			m_listDeadMotion.pop_back();
		}
	}
	return S_OK;
}

HRESULT CPlayer::Ready_Components()
{
	/* For.Com_Transform */
	CTransform::TRANSFORMDESC	_Desc;
	_Desc.fRotationPerSec = XMConvertToRadians(90.f);
	//_Desc.fSpeedPerSec = 1.5f;
	_Desc.fSpeedPerSec = 4.f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &_Desc)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_AnimModel"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Player"), TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Status */
	CStatus::STATUS _tStatus;
	_tStatus.fMaxHp = 200.f;
	_tStatus.fAttack = 30.f;
	_tStatus.fHp = _tStatus.fMaxHp;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Status"), TEXT("Com_Status"), (CComponent**)&m_pStatusCom, &_tStatus)))
		return E_FAIL;

	/* For.Com_Navigation */
	CNavigation::NAVIGATIONDESC			NaviDesc;
	ZeroMemory(&NaviDesc, sizeof(CNavigation::NAVIGATIONDESC));
	NaviDesc.iCurrentIndex = 0;
	switch (g_eCurLevel)
	{
	case Client::LEVEL_GAMEPLAY:
		if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Navigation_GamePlay"), TEXT("Com_Navigation"), (CComponent**)&m_pNavigationCom, &NaviDesc)))
			return E_FAIL;
		break;
	case Client::LEVEL_STAGE_02_1:
		if (FAILED(__super::Add_Component(LEVEL_STAGE_02_1, TEXT("Prototype_Component_Navigation_Stage_02_1"), TEXT("Com_Navigation"), (CComponent**)&m_pNavigationCom, &NaviDesc)))
			return E_FAIL;
		break;
	case Client::LEVEL_STAGE_02:
		
		break;
	case Client::LEVEL_STAGE_LAST:
		if (FAILED(__super::Add_Component(LEVEL_STAGE_LAST, TEXT("Prototype_Component_Navigation_Stage_Last"), TEXT("Com_Navigation"), (CComponent**)&m_pNavigationCom, &NaviDesc)))
			return E_FAIL;
		break;
	case Client::LEVEL_STAGE_LOBBY:
		if (FAILED(__super::Add_Component(LEVEL_STAGE_LOBBY, TEXT("Prototype_Component_Navigation_Stage_Lobby"), TEXT("Com_Navigation"), (CComponent**)&m_pNavigationCom, &NaviDesc)))
			return E_FAIL;
		break;
	}

	return S_OK;
}

void CPlayer::Add_Render()
{
	if (nullptr == m_pRendererCom || !m_bRender)
		return;

	switch (m_eWeapon)
	{
	case Client::CPlayer::WEAPON_NONE:
		break;
	case Client::CPlayer::WEAPON_BASE:
		for (auto& _pPart : m_pBaseParts)
		{
			m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, _pPart);
			//m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_SHADOW, _pPart);
		}
		break;
	case Client::CPlayer::WEAPON_SKILL:
		for (auto& _pPart : m_pSkillParts[m_eCurSkill])
		{
			m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, _pPart);
		}
		break;
	}

	for (auto& _motion : m_listMotion)
	{
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, _motion);
	}
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);
}

HRESULT CPlayer::Ready_AnimLimit()
{
	//공격
	m_vecLimitTime[STATE_ATT1].push_back(10.f);
	m_vecLimitTime[STATE_ATT1].push_back(20.f);
	m_vecLimitTime[STATE_ATT1].push_back(10.f);
	m_vecLimitTime[STATE_ATT1].push_back(25.f);
	m_vecLimitTime[STATE_ATT1].push_back(10.f);
	m_vecLimitTime[STATE_ATT1].push_back(25.f);

	m_vecLimitTime[STATE_ATT2].push_back(30.f);
	m_vecLimitTime[STATE_ATT2].push_back(20.f);
	m_vecLimitTime[STATE_ATT2].push_back(10.f);
	m_vecLimitTime[STATE_ATT2].push_back(25.f);
	m_vecLimitTime[STATE_ATT2].push_back(10.f);
	m_vecLimitTime[STATE_ATT2].push_back(25.f);

	m_vecLimitTime[STATE_ATT3].push_back(30.f);
	m_vecLimitTime[STATE_ATT3].push_back(25.f);
	m_vecLimitTime[STATE_ATT3].push_back(10.f);
	m_vecLimitTime[STATE_ATT3].push_back(25.f);
	m_vecLimitTime[STATE_ATT3].push_back(10.f);
	m_vecLimitTime[STATE_ATT3].push_back(25.f);

	m_vecLimitTime[STATE_ATT4].push_back(40.f);
	m_vecLimitTime[STATE_ATT4].push_back(40.f);
	m_vecLimitTime[STATE_ATT4].push_back(10.f);
	m_vecLimitTime[STATE_ATT4].push_back(40.f);
	m_vecLimitTime[STATE_ATT4].push_back(10.f);
	m_vecLimitTime[STATE_ATT4].push_back(40.f);

	m_vecLimitTime[STATE_ATT5].push_back(20.f);
	m_vecLimitTime[STATE_ATT5].push_back(50.f);
	m_vecLimitTime[STATE_ATT5].push_back(20.f);
	m_vecLimitTime[STATE_ATT5].push_back(50.f);
	m_vecLimitTime[STATE_ATT5].push_back(20.f);
	m_vecLimitTime[STATE_ATT5].push_back(50.f);

	//도끼스킬
	m_vecLimitTime[Corvus_PW_Axe].push_back(40.f);
	m_vecLimitTime[Corvus_PW_Axe].push_back(40.f);
	m_vecLimitTime[Corvus_PW_Axe].push_back(50.f);
	m_vecLimitTime[Corvus_PW_Axe].push_back(65.f);
	m_vecLimitTime[Corvus_PW_Axe].push_back(85.f);
	m_vecLimitTime[Corvus_PW_Axe].push_back(120.f);
	m_vecLimitTime[Corvus_PW_Axe].push_back(160.f);

	//쌍검
	m_vecLimitTime[DualKnife].push_back(5.f);
	m_vecLimitTime[DualKnife].push_back(40.f);

	//클로
	m_vecLimitTime[Raven_ClawNear].push_back(5.f);
	m_vecLimitTime[Raven_ClawNear].push_back(25.f);
	m_vecLimitTime[Raven_ClawNear].push_back(30.f);
	m_vecLimitTime[Raven_ClawNear].push_back(50.f);

	//패리
	m_vecLimitTime[ParryL].push_back(0.f);
	m_vecLimitTime[ParryL].push_back(90.f);

	//회피
	m_vecLimitTime[STATE_AVOIDATTACK].push_back(30.f);

	//임시
	m_vecLimitTime[Strong_Jump2].push_back(40.f);

	//창
	m_vecLimitTime[Corvus_PW_Halberds].push_back(10.f); // 트레일 on
	m_vecLimitTime[Corvus_PW_Halberds].push_back(55.f); // 이펙트 생성
	m_vecLimitTime[Corvus_PW_Halberds].push_back(90.f); // 충돌체 on
	m_vecLimitTime[Corvus_PW_Halberds].push_back(105.f); // 이펙트 생성
	m_vecLimitTime[Corvus_PW_Halberds].push_back(120.f); // 트레일 off, 충돌체 off
	m_vecLimitTime[Corvus_PW_Halberds].push_back(190.f); // 트레일 on
	m_vecLimitTime[Corvus_PW_Halberds].push_back(220.f); // 트레일 off

	//사이드
	m_vecLimitTime[Corvus_PW_Scythe].push_back(60.f); // 트레일 on, 충돌체 on
	m_vecLimitTime[Corvus_PW_Scythe].push_back(90.f);// 트레일 off, 충돌체 off

	//활
	m_vecLimitTime[PW_Bow_Start].push_back(50.f); // 슛

	m_vecLimitTime[Corvus_VSMagician_Execution].push_back(45.f);
	m_vecLimitTime[Corvus_VSMagician_Execution].push_back(65.f);

	m_vecLimitTime[Corvus_VSBossBat_Execution01].push_back(290.f);
	m_vecLimitTime[Corvus_VSBossBat_Execution01].push_back(330.f);

	m_vecLimitTime[Corvus_VSLV2Villager_M_Execution].push_back(0.f);
	m_vecLimitTime[Corvus_VSLV2Villager_M_Execution].push_back(30.f);
	m_vecLimitTime[Corvus_VSLV2Villager_M_Execution].push_back(50.f);

	
	return S_OK;
}

HRESULT CPlayer::Ready_Collider()
{
	/* For.Com_OBB */
	CCollider::COLLIDERDESC		ColliderDesc;
	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));

	ColliderDesc.vSize = _float3(0.7f, 1.4f, 0.7f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"), TEXT("Com_OBB"), (CComponent**)&m_pColliderCom[COLLIDERTYPE_BODY], &ColliderDesc)))
		return E_FAIL;

	ColliderDesc.vSize = _float3(0.7f, 1.4f, 0.7f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_Capsule"), TEXT("Com_Capsule"), (CComponent**)&m_pColliderCom[COLLIDERTYPE_PUSH], &ColliderDesc)))
		return E_FAIL;

	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));

	ColliderDesc.vSize = _float3(0.5f, 0.2f, 0.5f);
	_float3 vCenter = _float3(m_pModelCom->Get_HierarchyNode("ik_hand_gun")->Get_Trans()._41,
		m_pModelCom->Get_HierarchyNode("ik_hand_gun")->Get_Trans()._42,
		m_pModelCom->Get_HierarchyNode("ik_hand_gun")->Get_Trans()._43);
	ColliderDesc.vCenter = vCenter;
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"), TEXT("Com_Claw"), (CComponent**)&m_pColliderCom[COLLIDERTYPE_CLAW], &ColliderDesc)))
		return E_FAIL;

	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));

	ColliderDesc.vSize = _float3(0.9f, 1.6f, 0.7f);
	ColliderDesc.vCenter = _float3(0.f, 0.8f/*ColliderDesc.vSize.y * 0.5f*/, 0.3f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"), TEXT("Com_Parry"), (CComponent**)&m_pColliderCom[COLLIDERTYPE_PARRY], &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPlayer::SetUp_ShaderResources()
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);
	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;



	RELEASE_INSTANCE(CGameInstance);

	return S_OK;
}

HRESULT CPlayer::Ready_Hands()
{
	if (nullptr == m_pModelCom)
		return E_FAIL;

	CHierarchyNode*		pWeaponSocket = m_pModelCom->Get_HierarchyNode("ik_hand_gun");
	//CHierarchyNode*		pWeaponSocket = m_pModelCom->Get_HierarchyNode("weapon_r");
	if (nullptr == pWeaponSocket)
		return E_FAIL;
	m_pHands[HAND_RIGHT] = pWeaponSocket;
	
	pWeaponSocket = m_pModelCom->Get_HierarchyNode("ik_hand_l");
	if (nullptr == pWeaponSocket)
		return E_FAIL;
	m_pHands[HAND_LEFT] = pWeaponSocket;

	pWeaponSocket = m_pModelCom->Get_HierarchyNode("weapon_r");
	if (nullptr == pWeaponSocket)
		return E_FAIL;
	m_pBones[WEAPON_R] = pWeaponSocket;

	pWeaponSocket = m_pModelCom->Get_HierarchyNode("weapon_l");
	if (nullptr == pWeaponSocket)
		return E_FAIL;
	m_pBones[WEAPON_L] = pWeaponSocket;

 	m_pNails[NAIL_ONE] = m_pModelCom->Get_HierarchyNode("Claw_Bone_04_End");
	Safe_AddRef(m_pNails[NAIL_ONE]);

	m_pNails[NAIL_TWO] = m_pModelCom->Get_HierarchyNode("Claw_Bone_08_End");
	Safe_AddRef(m_pNails[NAIL_TWO]);

	m_pNails[NAIL_THREE] = m_pModelCom->Get_HierarchyNode("Claw_Bone_12_End");
	Safe_AddRef(m_pNails[NAIL_THREE]);

	m_pNails[NAIL_FOUR] = m_pModelCom->Get_HierarchyNode("Claw_Bone_16_End");
	Safe_AddRef(m_pNails[NAIL_FOUR]);



	return S_OK;
}

HRESULT CPlayer::Ready_PlayerParts()
{
	if (FAILED(Ready_PlayerParts_Base()))
	{
		return E_FAIL;
	}
	if (FAILED(Ready_PlayerParts_Skill()))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CPlayer::Ready_PlayerParts_Skill()
{
	AUTOINSTANCE(CGameInstance, pGameInstance);

	CWeapon*		pGameObject = static_cast<CWeapon*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Weapon_Axe")));

	if (nullptr == pGameObject)
		return E_FAIL;
	m_pSkillParts[SKILL_AXE].push_back(pGameObject);
	m_pSkillHands[SKILL_AXE].push_back(HAND_LEFT);
	m_fSkillAtts[SKILL_AXE] = 40.f;

	pGameObject = static_cast<CWeapon*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Weapon_Knife")));

	if (nullptr == pGameObject)
		return E_FAIL;
	m_pSkillParts[SKILL_DUAL].push_back(pGameObject);
	m_pSkillHands[SKILL_DUAL].push_back(HAND_RIGHT);

	pGameObject = static_cast<CWeapon*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Weapon_Knife")));

	if (nullptr == pGameObject)
		return E_FAIL;
	m_pSkillParts[SKILL_DUAL].push_back(pGameObject);
	m_pSkillHands[SKILL_DUAL].push_back(HAND_LEFT);
	m_fSkillAtts[SKILL_DUAL] = 45.f;

	pGameObject = static_cast<CWeapon*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Weapon_Bow")));

	if (nullptr == pGameObject)
		return E_FAIL;
	m_pSkillParts[SKILL_BOW].push_back(pGameObject);
	m_pSkillHands[SKILL_BOW].push_back(HAND_LEFT);
	

	pGameObject = static_cast<CWeapon*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Weapon_Scythe")));

	if (nullptr == pGameObject)
		return E_FAIL;
	m_pSkillParts[SKILL_SCYTHE].push_back(pGameObject);
	m_pSkillHands[SKILL_SCYTHE].push_back(HAND_RIGHT);
	m_fSkillAtts[SKILL_SCYTHE] = 90.f;

	pGameObject = static_cast<CWeapon*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Weapon_Lence")));

	if (nullptr == pGameObject)
		return E_FAIL;
	m_pSkillParts[SKILL_LENCE].push_back(pGameObject);
	m_pSkillHands[SKILL_LENCE].push_back(HAND_RIGHT);
	m_fSkillAtts[SKILL_LENCE] = 75.f;

	return S_OK;
}

HRESULT CPlayer::Ready_PlayerParts_Base()
{
	AUTOINSTANCE(CGameInstance, pGameInstance);

	/* For.Sword */
	CWeapon*		pGameObject = static_cast<CWeapon*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Weapon_Saber")));

	if (nullptr == pGameObject)
		return E_FAIL;

	m_pBaseParts.push_back(pGameObject);
	m_pBaseHands.push_back(HAND_RIGHT);

	pGameObject = static_cast<CWeapon*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Weapon_Dagger")));

	if (nullptr == pGameObject)
		return E_FAIL;

	m_pBaseParts.push_back(pGameObject);
	m_pBaseHands.push_back(HAND_LEFT);

	return S_OK;
}

HRESULT CPlayer::Update_Hands_Matrix()
{
	if (nullptr == m_pHands[HAND_RIGHT])
		return E_FAIL;

	_matrix WeaponMatrix = /*m_pSockets[PART_SABER]->Get_OffSetMatrix()**/
		m_pHands[HAND_RIGHT]->Get_CombinedTransformation()
		* XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())
		* m_pTransformCom->Get_WorldMatrix();

	XMStoreFloat4x4(&(m_matHands[HAND_RIGHT]), WeaponMatrix);

	if (nullptr == m_pHands[HAND_LEFT])
		return E_FAIL;

	WeaponMatrix = /*m_pSockets[PART_SABER]->Get_OffSetMatrix()**/
		m_pHands[HAND_LEFT]->Get_CombinedTransformation()
		* XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())
		* m_pTransformCom->Get_WorldMatrix();

	XMStoreFloat4x4(&(m_matHands[HAND_LEFT]), WeaponMatrix);

	return S_OK;
}

HRESULT CPlayer::Update_Weapon(_float fTimeDelta)
{
	if (FAILED(Update_Hands_Matrix()))
	{
		MSG_BOX(TEXT("failed to update Hands"));
		return E_FAIL;
	}

	switch (m_eWeapon)
	{
	case Client::CPlayer::WEAPON_NONE:
		return S_OK;
	case Client::CPlayer::WEAPON_BASE:
		if (FAILED(Update_Weapon_Base()))
		{
			MSG_BOX(TEXT("failed to update BaseWeapon"));
			return E_FAIL;
		}
		for (auto& pPart : m_pBaseParts)
		{
			if (pPart != nullptr)
				pPart->Tick(fTimeDelta, this);
		}
		return S_OK;
	case Client::CPlayer::WEAPON_SKILL:
		if (FAILED(Update_Weapon_Skill()))
		{
			MSG_BOX(TEXT("failed to update SkillWeapon"));
			return E_FAIL;
		}
		for (auto& pPart : m_pSkillParts[m_eCurSkill])
		{
			if (pPart != nullptr)
				pPart->Tick(fTimeDelta, this);
		}
		return S_OK;
	}

	return S_OK;
}

HRESULT CPlayer::Update_Weapon_Base()
{
	_uint i = 0;
	for (auto& _pPart : m_pBaseParts)
	{
		_pPart->SetUp_State(XMLoadFloat4x4(&(m_matHands[i])));
		++i;
	}

	return S_OK;
}

HRESULT CPlayer::Update_Weapon_Skill()
{
	_uint i = 0;
	for (auto& _pPart : m_pSkillParts[m_eCurSkill])
	{
		_pPart->SetUp_State(XMLoadFloat4x4(&(m_matHands[m_pSkillHands[m_eCurSkill][i]])));
		++i;
	}

	return S_OK;
}

HRESULT CPlayer::Change_Hand()
{
	CHierarchyNode* _TempHands[2];

	_TempHands[0] = m_pBones[0];
	_TempHands[1] = m_pBones[1];

	m_pBones[0] = m_pHands[0];
	m_pBones[1] = m_pHands[1];

	m_pHands[0] = _TempHands[0];
	m_pHands[1] = _TempHands[1];

	return S_OK;
}

CPlayer * CPlayer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayer*		pInstance = new CPlayer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CPlayer::Clone(void * pArg)
{
	CPlayer*		pInstance = new CPlayer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CPlayer"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayer::Free()
{
	__super::Free();

	for (auto& _Motion : m_listMotion)
	{
		Safe_Release(_Motion);
	}

	for (auto& _Motion : m_listDeadMotion)
	{
		Safe_Release(_Motion);
	}


	for (auto& _pHand : m_pHands)
	{
		if (_pHand)
			Safe_Release(_pHand);
	}
	for (auto& _pBone : m_pBones)
	{
		if (_pBone)
			Safe_Release(_pBone);
	}
	for (auto& _Part : m_pBaseParts)
	{
		if (_Part)
			Safe_Release(_Part);
	}
	for (_int i = 0; i < SKILL_END; ++i)
	{
		for (auto& _pPart : m_pSkillParts[i])
		{
			if (_pPart)
				Safe_Release(_pPart);
		}
	}

	for (auto& _pCollider : m_pColliderCom)
	{
		if (_pCollider)
			Safe_Release(_pCollider);
	}

	for (_uint i = 0 ; i < NAIL_END ; ++i)
	{
		Safe_Release(m_pNails[i]);
		Safe_Release(m_pClawTrail[i]);
	}

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pTransformCom);
	Safe_Release(m_pStatusCom);
	Safe_Release(m_pTarget);
	Safe_Release(m_pNavigationCom);
	//CGameInstance::Get_Instance()->Set_Player(nullptr);
}