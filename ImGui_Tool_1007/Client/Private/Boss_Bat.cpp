#include "stdafx.h"
#include "..\Public\Boss_Bat.h"
#include "GameInstance.h"
#include "ImGuiMgr.h"
#include "HierarchyNode.h"
#include "Weapon.h"
#include "CollisionMgr.h"
#include "Player.h"
#include "Status.h"
#include "Animation.h"
#include "CameraMgr.h"
#include "Camera.h"
#include "Camera_CutScene_Enter.h"

#include "Effect_Mgr.h"

#include "Bat_Claw.h"

#include "Bat_Dummy.h"
#include "MotionTrail.h"

#include "CrossTrail.h"

CBoss_Bat::CBoss_Bat(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CMonster(pDevice, pContext)
{
}

CBoss_Bat::CBoss_Bat(const CBoss_Bat & rhs)
	: CMonster(rhs)
	, m_AnimPos(rhs.m_AnimPos)
	, m_PreAnimPos(rhs.m_PreAnimPos)
{
}

HRESULT CBoss_Bat::Initialize_Prototype()
{
	XMStoreFloat4(&m_AnimPos, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	m_PreAnimPos = m_AnimPos;

	m_bAnimStop = true;

	return S_OK;
}

HRESULT CBoss_Bat::Initialize(void * pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	Ready_LimitTime();

	m_pEyes[0] = m_pModelCom->Get_HierarchyNode("ear_l");
	m_pEyes[1] = m_pModelCom->Get_HierarchyNode("ear_r");

	for (_uint i = 0; i < 2; ++i)
	{
		Safe_AddRef(m_pEyes[i]);
	}

	CCrossTrail::CROSS_DESC _tInfo;
	_tInfo.vRGBA = CLIENT_RGB(255.f, 0.f, 0.f);
	_tInfo.fWidth = 0.05f;
	_tInfo.bLook = true;
	m_pTrail[0] = static_cast<CCrossTrail*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_CROSSTRAIL, &(_tInfo)));
	m_pTrail[1] = static_cast<CCrossTrail*>(CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_CROSSTRAIL, &(_tInfo)));

	m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(30.672f, 0.f, 50.622f, 1.f));
	_bool		isMove = true;
	Get_AnimMat();
	_vector		vNormal = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	if (nullptr != m_pNavigationCom)
		isMove = m_pNavigationCom->isMove(XMVectorSet(30.672f, 0.f, 50.622f, 1.f), &vNormal);

	if (true == isMove)
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(30.672f, 0.f, 50.622f, 1.f));
	m_eCurState = BossBat_IdleBeforeFight;

	On_Collider(COLLIDERTYPE_BODY,true);
	return S_OK;
}

void CBoss_Bat::Tick(_float fTimeDelta)
{
	AUTOINSTANCE(CGameInstance, _Instance);

	if (m_bDead)
	{
		m_eMonsterState = ATTACK_DEAD;
		return;
	}
	if (m_pStatusCom->Get_Hp() <= 0.f && m_bCutScene == false)
	{
		//컷씬 - 시간도 느려지고
		AUTOINSTANCE(CCameraMgr, _pCamera);
		_pCamera->Change_Camera(CCameraMgr::CAMERA_CUTSCENE);
		m_bCutScene = true;
		//_Instance->Set_TimeSpeed(TEXT("Timer_Main"), 0.5f);
		CPlayer* _pPlayer = static_cast<CPlayer*>(_Instance->Get_Player());
		m_eCurState = BossBat_TakeExecution_DeadStart01;
		m_pModelCom->DirectAnim(BossBat_TakeExecution_DeadStart01);

		_pPlayer->Set_AnimState(CPlayer::Corvus_VSBossBat_Execution01);
		CAnimModel* _pModelCom = static_cast<CAnimModel*>(_pPlayer->Get_ComponentPtr(TEXT("Com_Model")));

		CTransform* _TargetTrans = static_cast<CTransform*>(_pPlayer->Get_ComponentPtr(TEXT("Com_Transform")));
		m_pTransformCom->LookAt_ForLandObject(_TargetTrans->Get_State(CTransform::STATE_POSITION));
		//_TargetTrans->LookAt_ForLandObject(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
		_pModelCom->DirectAnim(CPlayer::Corvus_VSBossBat_Execution01);
		_vector _vAnimPoint =
			(m_pModelCom->Get_HierarchyNode("AnimTargetPoint")->Get_CombinedTransformation()
				*XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())*m_pTransformCom->Get_WorldMatrix()).r[3];
		_TargetTrans->Set_State(CTransform::STATE_POSITION, XMVectorSetW(_vAnimPoint, 1.f));
		_TargetTrans->LookAt_ForLandObject(m_pTransformCom->Get_State(CTransform::STATE_POSITION));
		m_eMonsterState = ATTACK_DEAD;
	}
	
	_matrix WeaponMatrix = /*m_pSockets[PART_CANE]->Get_OffSetMatrix()**/
		m_pEyes[0]->Get_CombinedTransformation()
		* XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())
		* m_pTransformCom->Get_WorldMatrix();
	_float3 _vPos;
	WeaponMatrix.r[3] += XMVector3Normalize(WeaponMatrix.r[1]) * 0.5f;
	XMStoreFloat3(&_vPos, WeaponMatrix.r[3]);
	m_pTrail[0]->Add_Point(_vPos);


	WeaponMatrix = /*m_pSockets[PART_CANE]->Get_OffSetMatrix()**/
		m_pEyes[1]->Get_CombinedTransformation()
		* XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())
		* m_pTransformCom->Get_WorldMatrix();
	WeaponMatrix.r[3] += XMVector3Normalize(WeaponMatrix.r[1]) * 0.5f;
	XMStoreFloat3(&_vPos, WeaponMatrix.r[3]);
	m_pTrail[1]->Add_Point(_vPos);

	if (m_pModelCom != nullptr)
	{
		Check_Stun();
		CheckAnim();

		
		PlayAnimation(fTimeDelta);
		CheckState(fTimeDelta);
	}

	m_fHitCurTime += fTimeDelta;
	if (m_fHitMaxTime >= m_fHitCurTime)
	{
		On_Collider(COLLIDERTYPE_BODY, false);
	}
	else
	{
		On_Collider(COLLIDERTYPE_BODY, true);
		m_iPass = 0;
	}
	

	Update_Collider();

	for (auto iter = m_listMotion.begin(); iter != m_listMotion.end();)
	{
		(*iter)->Tick(fTimeDelta);
		if ((*iter)->Get_Tick() > 1.f)
		{
			(*iter)->Set_Tick(0.f);
			CBat_Dummy* _Motion = (*iter);
			iter = m_listMotion.erase(iter);
			m_listDeadMotion.push_back(_Motion);
		}
		else
			++iter;
	}
}

void CBoss_Bat::LateTick(_float fTimeDelta)
{
	if (m_bDead)
	{
		RenderGroup();
		return;
	}

	Collision(fTimeDelta);


	RenderGroup();
}

HRESULT CBoss_Bat::Render()
{
	if (nullptr == m_pModelCom ||
		nullptr == m_pShaderCom)
		return E_FAIL;



	SetUp_ShaderResources();

	_uint		iNumMeshes = m_pModelCom->Get_NumMesh();//메쉬 갯수를 알고 메쉬 갯수만큼 렌더를 할 것임. 여기서!

	for (_uint i = 0; i < iNumMeshes; ++i)
	{
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_DIFFUSE, "g_DiffuseTexture")))
			return E_FAIL;
		if (FAILED(m_pModelCom->SetUp_OnShader(m_pShaderCom, m_pModelCom->Get_MaterialIndex(i), aiTextureType_NORMALS, "g_NormalTexture")))
			return E_FAIL;


		if (FAILED(m_pModelCom->Render(m_pShaderCom, m_iPass, i)))
			return E_FAIL;
	}

	/*for (_uint i = 0; i < COLLILDERTYPE_END; ++i)
	{
		if (nullptr != m_pColliderCom[i])
			m_pColliderCom[i]->Render();
	}*/


	return S_OK;
}

HRESULT CBoss_Bat::Render_Shadow()
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

void CBoss_Bat::PlayAnimation(_float fTimeDelta)
{
	if (m_bAnimStop)
		return;
	_float4 _vAnim;
	XMStoreFloat4(&_vAnim, XMVectorSet(0.f, 0.f, 0.f, 1.f));

	if (m_bJumpAnim)
	{
		if (m_pModelCom->Play_Animation(fTimeDelta * m_fPlaySpeed, &_vAnim, &m_fPlayTime, m_bAgainAnim, "Bip001-Pelvis"))
		{
			CheckEndAnim();
		}
		CheckLimit();

		_vector _vPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

		_float Y = _vAnim.y;
		_vPos.m128_f32[1] = 0.f;
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);

		XMStoreFloat4(&m_AnimPos, (XMLoadFloat4(&_vAnim) - XMLoadFloat4(&m_PreAnimPos)));

		m_AnimPos.y = Y;
		m_PreAnimPos = _vAnim;
	}
	else
	{
		if (m_pModelCom->Play_Animation(fTimeDelta * m_fPlaySpeed, &_vAnim, &m_fPlayTime, m_bAgainAnim))
		{
			CheckEndAnim();
		}
		CheckLimit();

		XMStoreFloat4(&m_AnimPos, (XMLoadFloat4(&_vAnim) - XMLoadFloat4(&m_PreAnimPos)));

		m_PreAnimPos = _vAnim;
	}
}


void CBoss_Bat::CheckEndAnim()
{
	m_fPlaySpeed = 1.f;
	m_iPass = 0;
	m_bEffect = false;
	switch (m_eCurState)
	{
	case Client::CBoss_Bat::BossBat_AttackL_01_1:
		m_eCurState = BossBat_Idle;
		//m_eCurState = BossBat_AttackL_01_1;
		break;
	case Client::CBoss_Bat::BossBat_AttackL_01_2b:
		m_eCurState = BossBat_Idle;
		
		break;
	case Client::CBoss_Bat::BossBat_AttackL_01_3a:
		m_eCurState = BossBat_Idle;
		//m_eCurState = BossBat_AttackL_01_3a;
		break;
	case Client::CBoss_Bat::BossBat_AttackR_01_1:
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_AttackR_01_2b:
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_Bite_2:
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_Dash:
		//CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER)->Shake_Off();
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_FTurn_L:
		m_pModelCom->DirectAnim(BossBat_Idle);
		m_pTransformCom->Turn_Angle(XMVectorSet(0.f,1.f,0.f,0.f),XMConvertToRadians(180.f));
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_FTurn_R:
		m_pModelCom->DirectAnim(BossBat_Idle);
		m_pTransformCom->Turn_Angle(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(180.f));
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_HurtXL_R:
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_HurtXL_L:
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_Idle:
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_JumpSmash_Chest:
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_JumpSmashForwardL:
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_JumpSmashL:
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_FightStart:
		//m_eCurState = BossBat_IdleBeforeFight;
		//m_eCurState = BossBat_Idle;
		{
			AUTOINSTANCE(CCameraMgr, _pCamMgr);
			_pCamMgr->Change_Camera(CCameraMgr::CAMERA_PLAYER);
			//m_eCurState = BossBat_Idle;
			m_eCurState = BossBat_Idle;
		}
		break;
	case Client::CBoss_Bat::BossBat_Stun:
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_TakeExecution_End:
		m_bDead = true;
		break;
	case Client::CBoss_Bat::BossBat_TakeExecution_DeadStart01:
		m_eCurState = BossBat_TakeExecution_End;
		break;
	case Client::CBoss_Bat::BossBat_TurnL90:
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_TurnR90:
		m_eCurState = BossBat_Idle;
		break;
	case Client::CBoss_Bat::BossBat_WalkF:
		break;
	}

	XMStoreFloat4(&m_AnimPos, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	m_PreAnimPos = m_AnimPos;
}

void CBoss_Bat::CheckState(_float fTimeDelta)
{
	switch (m_eCurState)
	{
	case Client::CBoss_Bat::BossBat_AttackL_01_1:
		break;
	case Client::CBoss_Bat::BossBat_AttackL_01_2b:
		break;
	case Client::CBoss_Bat::BossBat_AttackL_01_3a:
		break;
	case Client::CBoss_Bat::BossBat_AttackR_01_1:
		break;
	case Client::CBoss_Bat::BossBat_AttackR_01_2b:
		break;
	case Client::CBoss_Bat::BossBat_Bite_2:
		break;
	case Client::CBoss_Bat::BossBat_Dash:
		break;
	case Client::CBoss_Bat::BossBat_FTurn_L:
		break;
	case Client::CBoss_Bat::BossBat_FTurn_R:
		break;
	case Client::CBoss_Bat::BossBat_HurtXL_R:
		break;
	case Client::CBoss_Bat::BossBat_HurtXL_L:
		break;
	case Client::CBoss_Bat::BossBat_Idle:
		On_Collider(COLLIDERTYPE_ATTBODY, false);
		Turn();
		break;
	case Client::CBoss_Bat::BossBat_JumpSmash_Chest:
		break;
	case Client::CBoss_Bat::BossBat_JumpSmashForwardL:
		break;
	case Client::CBoss_Bat::BossBat_JumpSmashL:
		break;
	case Client::CBoss_Bat::BossBat_FightStart:
		break;
	case Client::CBoss_Bat::BossBat_Stun:
		break;
	case Client::CBoss_Bat::BossBat_TakeExecution_End:
		break;
	case Client::CBoss_Bat::BossBat_TakeExecution_DeadStart01:
		break;
	case Client::CBoss_Bat::BossBat_TurnL90:
		break;
	case Client::CBoss_Bat::BossBat_TurnR90:
		break;
	case Client::CBoss_Bat::BossBat_WalkF:
		Turn();
		m_pTransformCom->Go_Straight(fTimeDelta / 2.f);
		break;
	case Client::CBoss_Bat::BossBat_IdleBeforeFight:
	{
		AUTOINSTANCE(CCameraMgr, _pCameraMgr);
		if (static_cast<CCamera_CutScene_Enter*>(_pCameraMgr->Get_Cam(CCameraMgr::CAMERA_CUTSCENE_ENTER))->Get_CutSceneNum() == 1)
			m_eCurState = BossBat_FightStart;
	}
		break;
	}
	Get_AnimMat();
	
}

void CBoss_Bat::CheckLimit()
{
	m_bJumpAnim = false;

	switch (m_eCurState)
	{
	case Client::CBoss_Bat::BossBat_AttackL_01_1:
		if (m_vecLimitTime[BossBat_AttackL_01_1][2] < m_fPlayTime)
		{
			//On_Collider(COLLIDERTYPE_BODY, true);
			On_Collider(COLLIDERTYPE_HAND_L, false);
			m_fPlaySpeed = 1.f;
		}
		else if (m_vecLimitTime[BossBat_AttackL_01_1][1] < m_fPlayTime)
		{
			//if (!m_bEffect)
			{
				CBat_Claw::CLAW_DESC _tInfo;
				_tInfo.TargetMatrix = m_pTransformCom->Get_WorldFloat4x4();
				_tInfo.eClaw = CBat_Claw::CLAWTYPE_LEFT;
				XMStoreFloat3(&_tInfo.vDir, m_pTransformCom->Get_State(CTransform::STATE_LOOK));
				_tInfo.vDis = { -2.f,2.f,3.5f };
				_tInfo.vAngle = { 0.f,0.f,-60.f };
				CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_BATCLAW, &_tInfo);
				m_bEffect = true;
			}

		}
		else if (m_vecLimitTime[BossBat_AttackL_01_1][0] < m_fPlayTime)
		{
			//On_Collider(COLLIDERTYPE_BODY, false);
			On_Collider(COLLIDERTYPE_HAND_L, true);
			m_fPlaySpeed = 3.f;
		}
		break;
	case Client::CBoss_Bat::BossBat_AttackL_01_2b:
		if (m_vecLimitTime[BossBat_AttackL_01_2b][2] < m_fPlayTime)
		{
			//On_Collider(COLLIDERTYPE_BODY, true);
			On_Collider(COLLIDERTYPE_HAND_L, false);
			m_fPlaySpeed = 1.f;
		}
		else if (m_vecLimitTime[BossBat_AttackL_01_2b][1] < m_fPlayTime)
		{
		//	if (!m_bEffect)
			{
				CBat_Claw::CLAW_DESC _tInfo;
				_tInfo.TargetMatrix = m_pTransformCom->Get_WorldFloat4x4();
				_tInfo.eClaw = CBat_Claw::CLAWTYPE_LEFT;
				XMStoreFloat3(&_tInfo.vDir, m_pTransformCom->Get_State(CTransform::STATE_LOOK));
				_tInfo.vDis = { -2.f,3.9f,3.5f };
				_tInfo.vAngle = { 130.f,0.f,-30.f };
				CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_BATCLAW, &_tInfo);
				m_bEffect = true;
			}
		}
		else if (m_vecLimitTime[BossBat_AttackL_01_2b][0] < m_fPlayTime)
		{
			//On_Collider(COLLIDERTYPE_BODY, false);
			On_Collider(COLLIDERTYPE_HAND_L, true);
			m_fPlaySpeed = 3.f;
		}
		break;
	case Client::CBoss_Bat::BossBat_AttackL_01_3a:
		if (m_vecLimitTime[BossBat_AttackL_01_3a][2] < m_fPlayTime)
		{
			//On_Collider(COLLIDERTYPE_BODY, true);
			On_Collider(COLLIDERTYPE_HAND_R, false);
			m_fPlaySpeed = 1.f;
		}
		else if (m_vecLimitTime[BossBat_AttackL_01_3a][1] < m_fPlayTime)
		{
		//	if (!m_bEffect)
			{
				CBat_Claw::CLAW_DESC _tInfo;
				_tInfo.TargetMatrix = m_pTransformCom->Get_WorldFloat4x4();
				_tInfo.eClaw = CBat_Claw::CLAWTYPE_LEFT;
				XMStoreFloat3(&_tInfo.vDir, m_pTransformCom->Get_State(CTransform::STATE_LOOK));
				_tInfo.vDis = { 3.5f,2.f,4.f };
				_tInfo.vAngle = { 0.f,0.f,-25.f };
				CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_BATCLAW, &_tInfo);
				m_bEffect = true;
			}
		}
		else if (m_vecLimitTime[BossBat_AttackL_01_3a][0] < m_fPlayTime)
		{
			//On_Collider(COLLIDERTYPE_BODY, false);
			On_Collider(COLLIDERTYPE_HAND_R, true);
			m_fPlaySpeed = 3.f;
		}
		break;
	case Client::CBoss_Bat::BossBat_AttackR_01_1:
		if (m_vecLimitTime[BossBat_AttackR_01_1][2] < m_fPlayTime)
		{
			CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER)->Shake_Off();
			//On_Collider(COLLIDERTYPE_BODY, true);
			On_Collider(COLLIDERTYPE_HAND_R, false);
			m_fPlaySpeed = 1.f;
		}
		else if (m_vecLimitTime[BossBat_AttackR_01_1][1] < m_fPlayTime)
		{
		//	if (!m_bEffect)
			{
				CBat_Claw::CLAW_DESC _tInfo;
				_tInfo.TargetMatrix = m_pTransformCom->Get_WorldFloat4x4();
				_tInfo.eClaw = CBat_Claw::CLAWTYPE_LEFT;
				XMStoreFloat3(&_tInfo.vDir, m_pTransformCom->Get_State(CTransform::STATE_LOOK));
				_tInfo.vDis = { 2.5f,2.f,4.5f };
				_tInfo.vAngle = { 0.f,0.f,-25.f };
				CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_BATCLAW, &_tInfo);
				m_bEffect = true;
			}
		}
		else if (m_vecLimitTime[BossBat_AttackR_01_1][0] < m_fPlayTime)
		{
			//On_Collider(COLLIDERTYPE_BODY, false);
			On_Collider(COLLIDERTYPE_HAND_R, true);
			CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER)->Shake_On(0.1f, 0.01f);
			m_fPlaySpeed = 3.f;
		}
		break;
	case Client::CBoss_Bat::BossBat_AttackR_01_2b:
		if (m_vecLimitTime[BossBat_AttackR_01_2b][2] < m_fPlayTime)
		{
			CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER)->Shake_Off();
			m_fPlaySpeed = 1.f;
			//On_Collider(COLLIDERTYPE_BODY, true);
			On_Collider(COLLIDERTYPE_HAND_R, false);
		}
		else if (m_vecLimitTime[BossBat_AttackR_01_2b][1] < m_fPlayTime)
		{
		//	if (!m_bEffect)
			{
				CBat_Claw::CLAW_DESC _tInfo;
				_tInfo.TargetMatrix = m_pTransformCom->Get_WorldFloat4x4();
				_tInfo.eClaw = CBat_Claw::CLAWTYPE_LEFT;
				XMStoreFloat3(&_tInfo.vDir, m_pTransformCom->Get_State(CTransform::STATE_LOOK));
				_tInfo.vDis = { 4.f,3.9f,3.5f };
				_tInfo.vAngle = { 130.f,0.f,-50.f };
				CEffect_Mgr::Get_Instance()->Add_Effect(CEffect_Mgr::EFFECT_BATCLAW, &_tInfo);
				m_bEffect = true;
			}
		}
		else if (m_vecLimitTime[BossBat_AttackR_01_2b][0] < m_fPlayTime)
		{
			CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER)->Shake_On(3.5f, 5.f);
			m_fPlaySpeed = 3.f;
			//On_Collider(COLLIDERTYPE_BODY, false);
			On_Collider(COLLIDERTYPE_HAND_R, true);
		}
		break;
	case Client::CBoss_Bat::BossBat_Bite_2:
		if (m_vecLimitTime[BossBat_Bite_2][2] < m_fPlayTime)
		{
			m_fPlaySpeed = 1.f;
			//On_Collider(COLLIDERTYPE_BODY, true);
			On_Collider(COLLIDERTYPE_MOUSE, false);
		}
		else if (m_vecLimitTime[BossBat_Bite_2][1] < m_fPlayTime)
		{
			if (!m_bEffect)
			{
				if (m_listDeadMotion.empty())
				{
					CMotionTrail* _Motion = CMotionTrail::Create(m_pDevice, m_pContext, m_pModelCom);
					if (_Motion == nullptr)
					{
						MSG_BOX(TEXT("creat Motion"));
						return;
					}
					_Motion->Set_CombinedMat(m_pModelCom->Get_HierarchyNodeVector(), m_pTransformCom->Get_WorldFloat4x4());
					CBat_Dummy*		_pDummy = CBat_Dummy::Creat(m_pDevice, m_pContext, _Motion, m_pTransformCom->Get_WorldFloat4x4());
					m_listMotion.push_back(_pDummy);
				}
				else
				{
					CBat_Dummy*		_pDummy = m_listDeadMotion.back();
					_pDummy->Set_CombinedMat(m_pModelCom->Get_HierarchyNodeVector(), m_pTransformCom->Get_WorldFloat4x4());
					m_listMotion.push_back(_pDummy);
					m_listDeadMotion.pop_back();
				}
				m_bEffect = true;
			}
		}
		else if (m_vecLimitTime[BossBat_Bite_2][0] < m_fPlayTime)
		{
			m_fPlaySpeed = 3.f;
			//On_Collider(COLLIDERTYPE_BODY, false);
			On_Collider(COLLIDERTYPE_MOUSE, true);
		}
		break;
	case Client::CBoss_Bat::BossBat_Dash:
		if (m_vecLimitTime[BossBat_Dash][1] < m_fPlayTime)
		{
			//m_fPlaySpeed = 3.f;
			//On_Collider(COLLIDERTYPE_BODY, true);
			On_Collider(COLLIDERTYPE_ATTBODY, false);
		}
		else if (m_vecLimitTime[BossBat_Dash][0] < m_fPlayTime)
		{
			CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER)->Shake_On(3.5f, 5.f);
			//m_fPlaySpeed = 3.f;
			//On_Collider(COLLIDERTYPE_BODY, false);
			On_Collider(COLLIDERTYPE_ATTBODY, true);
		}
		On_Collider(COLLIDERTYPE_ATTBODY, true);
		break;
	case Client::CBoss_Bat::BossBat_FTurn_L:
		if (m_vecLimitTime[BossBat_FTurn_L][1] < m_fPlayTime)
		{
			On_Collider(COLLIDERTYPE_BODY, true);
			m_bJumpAnim = false;

		}
		else if (m_vecLimitTime[BossBat_FTurn_L][0] < m_fPlayTime)
			m_bJumpAnim = true;
		{
			On_Collider(COLLIDERTYPE_BODY, false);
		}
		break;
	case Client::CBoss_Bat::BossBat_FTurn_R:
		if (m_vecLimitTime[BossBat_FTurn_R][1] < m_fPlayTime)
		{
			On_Collider(COLLIDERTYPE_BODY, true);
			m_bJumpAnim = false;

		}
		else if (m_vecLimitTime[BossBat_FTurn_R][0] < m_fPlayTime)
		{
			m_bJumpAnim = true;
			On_Collider(COLLIDERTYPE_BODY, false);
		}
		break;
	case Client::CBoss_Bat::BossBat_HurtXL_R:
		if (m_vecLimitTime[BossBat_HurtXL_R][1] < m_fPlayTime)
		{
			On_Collider(COLLIDERTYPE_BODY, true);
		}
		else if (m_vecLimitTime[BossBat_HurtXL_R][0] < m_fPlayTime)
		{
			On_Collider(COLLIDERTYPE_BODY, false);
		}
		break;
	case Client::CBoss_Bat::BossBat_HurtXL_L:
		if (m_vecLimitTime[BossBat_HurtXL_L][1] < m_fPlayTime)
		{
			On_Collider(COLLIDERTYPE_BODY, true);
		}
		else if (m_vecLimitTime[BossBat_HurtXL_L][0] < m_fPlayTime)
		{
			On_Collider(COLLIDERTYPE_BODY, false);
		}
		break;
	case Client::CBoss_Bat::BossBat_Idle:

		break;
	case Client::CBoss_Bat::BossBat_JumpSmash_Chest:
		if (m_vecLimitTime[BossBat_JumpSmash_Chest][2] < m_fPlayTime)
		{
			On_Collider(COLLIDERTYPE_BODY, true);
			On_Collider(COLLIDERTYPE_ATTBODY, false);
			m_fPlaySpeed = 1.f;
		}
		else if (m_vecLimitTime[BossBat_JumpSmash_Chest][1] < m_fPlayTime)
		{
			CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER)->Shake_On(0.5f, 5.f);
			m_bJumpAnim = false;

			On_Collider(COLLIDERTYPE_BODY, false);
			On_Collider(COLLIDERTYPE_ATTBODY, true);
			m_fPlaySpeed = 3.f;
		}
		else if (m_vecLimitTime[BossBat_JumpSmash_Chest][0] < m_fPlayTime)
		{
			m_bJumpAnim = true;
		}
		break;
	case Client::CBoss_Bat::BossBat_JumpSmashForwardL:
		if (m_vecLimitTime[BossBat_JumpSmashForwardL][3] < m_fPlayTime)
		{
			m_eCurState = BossBat_Idle;
		}
		else if (m_vecLimitTime[BossBat_JumpSmashForwardL][2] < m_fPlayTime)
		{
			//CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER)->Shake_On(0.5f, 5.f);

			On_Collider(COLLIDERTYPE_BODY, true);
			On_Collider(COLLIDERTYPE_ATTBODY, false);
			m_fPlaySpeed = 1.f;
		}
		else if (m_vecLimitTime[BossBat_JumpSmashForwardL][1] < m_fPlayTime)
		{
			CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER)->Shake_On(0.5f, 5.f);
			m_bJumpAnim = false;
			On_Collider(COLLIDERTYPE_BODY, false);
			On_Collider(COLLIDERTYPE_ATTBODY, true);
			m_fPlaySpeed = 3.f;
		}
		else if (m_vecLimitTime[BossBat_JumpSmashForwardL][0] < m_fPlayTime)
		{
			m_bJumpAnim = true;
		}
		break;
	case Client::CBoss_Bat::BossBat_JumpSmashL:
		if (m_vecLimitTime[BossBat_JumpSmashL][3] < m_fPlayTime)
		{
			m_eCurState = BossBat_Idle;
		}
		else if (m_vecLimitTime[BossBat_JumpSmashL][2] < m_fPlayTime)
		{
			CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER)->Shake_Off();

			m_fPlaySpeed = 1.f;
			On_Collider(COLLIDERTYPE_BODY, true);
			On_Collider(COLLIDERTYPE_ATTBODY, false);
		}
		else if (m_vecLimitTime[BossBat_JumpSmashL][1] < m_fPlayTime)
		{
			CCameraMgr::Get_Instance()->Get_Cam(CCameraMgr::CAMERA_PLAYER)->Shake_On(0.5f, 5.f);
			m_bJumpAnim = true;

			m_fPlaySpeed = 3.f;
			On_Collider(COLLIDERTYPE_BODY, false);
			On_Collider(COLLIDERTYPE_ATTBODY, true);

		}
		else if (m_vecLimitTime[BossBat_JumpSmashL][0] < m_fPlayTime)
		{
			m_bJumpAnim = false;
		}
		break;
	case Client::CBoss_Bat::BossBat_FightStart:
		break;
	case Client::CBoss_Bat::BossBat_Stun:
		break;
	case Client::CBoss_Bat::BossBat_TakeExecution_End:
		break;
	case Client::CBoss_Bat::BossBat_TakeExecution_DeadStart01:
		break;
	case Client::CBoss_Bat::BossBat_TurnL90:
		break;
	case Client::CBoss_Bat::BossBat_TurnR90:
		break;
	case Client::CBoss_Bat::BossBat_WalkF:
		break;
	}
}

void CBoss_Bat::Set_Anim(STATE _eState)
{
	m_eCurState = _eState;
	XMStoreFloat4(&m_AnimPos, XMVectorSet(0.f, 0.f, 0.f, 1.f));
	m_PreAnimPos = m_AnimPos;
	if (m_pStatusCom->Get_Hp() <= 0.f)
		m_eMonsterState = CMonster::ATTACK_DEAD;
	else
		m_eMonsterState = CMonster::ATTACK_IDLE;
	m_pModelCom->Set_AnimationIndex(m_eCurState);
	m_fPlayTime = 0.f;
}

void CBoss_Bat::CheckAnim()
{
	if (m_ePreState != m_eCurState)
	{
		
		Set_Anim(m_eCurState);
		m_ePreState = m_eCurState;
	}
}

void CBoss_Bat::Get_AnimMat()
{
	if (m_bAnimStop)
		return;
	_fmatrix _World = m_pTransformCom->Get_WorldMatrix();
	_vector _vPos;
	_vPos = XMVector3TransformCoord(XMLoadFloat4(&m_AnimPos), _World);

	_bool		isMove = true;

	if (nullptr != m_pNavigationCom)
		isMove = m_pNavigationCom->isMove(_vPos, nullptr);

	if (true == isMove)
	{
		if (m_eCurState == BossBat_WalkF || m_eCurState == BossBat_Idle)
			_vPos.m128_f32[1] = m_pNavigationCom->Get_PosY(_vPos);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vPos);
	}
}

void CBoss_Bat::RenderGroup()
{
	if (nullptr == m_pRendererCom)
		return;

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONALPHABLEND, this);
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_SHADOW, this);

	for (auto& _motion : m_listMotion)
	{
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_ALPHABLEND, _motion);
	}
}

_bool CBoss_Bat::Collision(_float fTimeDelta)
{
	CGameObject* _pTarget = nullptr;

	AUTOINSTANCE(CGameInstance, _instance);

	if ((_pTarget = m_pColliderCom[COLLIDERTYPE_BODY]->Get_Target()) && (CPlayer::ParryL != *static_cast<CPlayer*>(_pTarget)->Get_AnimState()))
	{
		if (TYPE_BULLET != _pTarget->Get_ObjType())
		{
			CPlayer* _pPlayer = static_cast<CPlayer*>(_pTarget);
			CPlayer::STATE _ePlayerState = *_pPlayer->Get_AnimState();
			if (_ePlayerState == CPlayer::Corvus_PW_Axe || _ePlayerState == CPlayer::Corvus_PW_Scythe ||
				_ePlayerState == CPlayer::Corvus_PW_Halberds || _ePlayerState == CPlayer::Raven_ClawNear)
			{
				m_iPass = 5;
			}
		}
		else
		{
			m_iPass = 5;
		}



		m_pStatusCom->Damage(static_cast<CStatus*>(_pTarget->Get_ComponentPtr(TEXT("Com_Status")))->Get_Attack());
		m_fHitCurTime = 0.f;
	}
	return false;
}

void CBoss_Bat::On_Collider(MAGICIANCOLLIDER _eCollider, _bool _bCollision)
{
	switch (_eCollider)
	{
	case Client::CBoss_Bat::COLLIDERTYPE_BODY:
		if (m_bCollision[COLLIDERTYPE_BODY] = _bCollision)
			CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_MONSTER_BODY, m_pColliderCom[COLLIDERTYPE_BODY], this);
		break;
	case Client::CBoss_Bat::COLLIDERTYPE_HAND_R:
		if (m_bCollision[COLLIDERTYPE_HAND_R] = _bCollision)
			CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_MONSTER_WEAPON, m_pColliderCom[COLLIDERTYPE_HAND_R], this);
		break;
	case Client::CBoss_Bat::COLLIDERTYPE_HAND_L:
		if (m_bCollision[COLLIDERTYPE_HAND_L] = _bCollision)
			CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_MONSTER_WEAPON, m_pColliderCom[COLLIDERTYPE_HAND_L], this);
		break;
	case Client::CBoss_Bat::COLLIDERTYPE_ATTBODY:
		if (m_bCollision[COLLIDERTYPE_ATTBODY] = _bCollision)
			CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_MONSTER_WEAPON, m_pColliderCom[COLLIDERTYPE_ATTBODY], this);
		break;
	}
}

void CBoss_Bat::Look_Move_Player(_float _fPosX, _float _fPosZ)
{
	// 애니메이션 진행 중에 턴하는 함수

	AUTOINSTANCE(CGameInstance, _Intance);
	CTransform* _Trans =
		static_cast<CTransform*>(_Intance->Get_Player()->Get_ComponentPtr(TEXT("Com_Transform")));

	CNavigation* _pNavi =
		static_cast<CNavigation*>(_Intance->Get_Player()->Get_ComponentPtr(TEXT("Com_Navigation")));
	m_pNavigationCom->Set_Index(_pNavi->Get_Index());
	_vector _vTargetPos = _Trans->Get_State(CTransform::STATE_POSITION);
	_vTargetPos.m128_f32[0] += _fPosX;
	_vTargetPos.m128_f32[2] += _fPosZ;

	_bool		isMove = true;
	_vector		vNormal = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	if (nullptr != m_pNavigationCom)
		isMove = m_pNavigationCom->isMove(_vTargetPos, &vNormal);

	if (true == isMove)
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _vTargetPos);

	m_pTransformCom->LookAt_ForLandObject(_Trans->Get_State(CTransform::STATE_POSITION));
}

void CBoss_Bat::Look_Player()
{
	//한번에 바라보는거
	AUTOINSTANCE(CGameInstance, _Intance);
	CTransform* _Trans =
		static_cast<CTransform*>(_Intance->Get_Player()->Get_ComponentPtr(TEXT("Com_Transform")));

	m_pTransformCom->LookAt_ForLandObject(_Trans->Get_State(CTransform::STATE_POSITION));
}

void CBoss_Bat::Turn()
{
	/*
	플레이어가 룩의 일정 범위 안에 있으면 턴 안하고
	일정 범위 안에 없으면 그 범위 안에 들어올때까지 턴한다
	*/

	AUTOINSTANCE(CGameInstance, _pInstance);
	CPlayer* _pPlayer = static_cast<CPlayer*>(_pInstance->Get_Player());

	_vector _vLook = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));

	CTransform* _pTransform = static_cast<CTransform*>(_pPlayer->Get_ComponentPtr(TEXT("Com_Transform")));
	_vector _vDestLook = XMVector3Normalize(_pTransform->Get_State(CTransform::STATE_POSITION) - m_pTransformCom->Get_State(CTransform::STATE_POSITION));

	_float _fAngle = XMVector3Dot(_vLook, _vDestLook).m128_f32[0];


	m_eCurState = BossBat_WalkF;

	m_pTransformCom->Turn(_vLook,
		_vDestLook
		, 0.8f);

	if (_fAngle > ((sqrtf(6.f) + sqrtf(2.f)) / 4.f))
		Pattern();

}

void CBoss_Bat::Pattern()
{
	//거리에 따라 확률적으로 공격하고 날라댕기고
	// 거리 짧으면 또 그러고 어키어키
	AUTOINSTANCE(CGameInstance, _pInstance);
	_uint chance = _pInstance->Rand_Int(1,100);

	CTransform* _pPlayerTransform = static_cast<CTransform*>(_pInstance->Get_Player()->Get_ComponentPtr(TEXT("Com_Transform")));
	_float _fDistance = XMVector3Length(_pPlayerTransform->Get_State(CTransform::STATE_POSITION) - m_pTransformCom->Get_State(CTransform::STATE_POSITION)).m128_f32[0];

	//공격인지 아이들인지 먼저 체크
	if (chance > 5 || _fDistance > 30.f)
	{
		return;
	}

	// 공격 패턴 짜기 시작
	//플레이어와의 거리
	
	if (_fDistance < 15.f)
	{
		_uint ChanceShort = _pInstance->Rand_Int(0, 6);
		switch (ChanceShort)
		{
		case 0:
			m_eCurState = BossBat_AttackL_01_1;
			break;
		case 1:
			m_eCurState = BossBat_AttackL_01_2b;
			break;
		case 2:
			m_eCurState = BossBat_AttackL_01_3a;
			break;
		case 3:
			m_eCurState = BossBat_AttackR_01_1;
			break;
		case 4:
			m_eCurState = BossBat_AttackR_01_2b;
			break;
		case 5:
			m_eCurState = BossBat_Bite_2;
			break;
		
		case 6:
			m_eCurState = BossBat_JumpSmashL;
			break;
		}
	}
	else
	{
		_uint ChanceShort = _pInstance->Rand_Int(0, 5);
		switch (ChanceShort)
		{
		case 0:
			m_eCurState = BossBat_JumpSmash_Chest;
			break;
		case 1:
			m_eCurState = BossBat_JumpSmashForwardL;
			break;
		case 2:
			m_eCurState = BossBat_Dash;
			break;
		case 3:
			m_eCurState = BossBat_FTurn_L;
			break;
		case 4:
			m_eCurState = BossBat_FTurn_R;
			break;
		case 5:
			m_eCurState = BossBat_Dash;
			break;
		}

	}
}

void CBoss_Bat::CurrentRot()
{
	// 그 순간의 회전값을 얻어와서 다시 행렬을 세팅.
	KEYFRAME _tOldKeyFrame = (*(m_pModelCom->Get_AllAnimation()))[BossBat_TurnL90]->Get_OldKeyFrame(m_pModelCom->Get_HierarchyNode("root")->Get_Index());
	_tOldKeyFrame.vScale = m_pTransformCom->Get_Scale();

	XMStoreFloat3(&_tOldKeyFrame.vPosition, m_pTransformCom->Get_State(CTransform::STATE_POSITION));

	_tOldKeyFrame.vRotation; // 쿼터니언으로 받아옴 - 새로운 회전 행렬을 만들어준다

	_matrix		TransformationMatrix = XMMatrixAffineTransformation(XMLoadFloat3(&_tOldKeyFrame.vScale), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMLoadFloat4(&_tOldKeyFrame.vRotation), XMVectorSetW(XMLoadFloat3(&_tOldKeyFrame.vPosition), 1.f));
	_float4x4	_World;
	XMStoreFloat4x4(&_World, TransformationMatrix);
	m_pTransformCom->Set_WorldFloat4x4(_World);
}

HRESULT CBoss_Bat::Ready_Components()
{
	AUTOINSTANCE(CGameInstance, _pInstance);

	/* For.Com_Transform */
	CTransform::TRANSFORMDESC	_Desc;
	_Desc.fRotationPerSec = XMConvertToRadians(90.f);
	//_Desc.fSpeedPerSec = 1.5f;
	_Desc.fSpeedPerSec = 2.5f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Transform"), TEXT("Com_Transform"), (CComponent**)&m_pTransformCom, &_Desc)))
		return E_FAIL;

	/* For.Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"), TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_AnimModel"), TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_STAGE_02, TEXT("Prototype_Component_Model_Monster_bat"), TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Status */
	CStatus::STATUS _tStatus;
	_tStatus.fMaxHp = 100.f;
	_tStatus.fAttack = 20.f;
	_tStatus.fHp = _tStatus.fMaxHp;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Status"), TEXT("Com_Status"), (CComponent**)&m_pStatusCom, &_tStatus)))
		return E_FAIL;


	CCollider::COLLIDERDESC		ColliderDesc;
	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));
	ColliderDesc.vSize = _float3(6.f, 5.f, 6.f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 2.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"), TEXT("Com_OBB"), (CComponent**)&m_pColliderCom[COLLIDERTYPE_BODY], &ColliderDesc)))
		return E_FAIL;

	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));
	ColliderDesc.vSize = _float3(6.f, 5.f, 6.f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 2.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"), TEXT("Com_OBB_AttBody"), (CComponent**)&m_pColliderCom[COLLIDERTYPE_ATTBODY], &ColliderDesc)))
		return E_FAIL;

	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));
	ColliderDesc.vSize = _float3(3.5f, 3.5f, 3.5f);
	_float3 vCenter = _float3(m_pModelCom->Get_HierarchyNode("middle_01_r")->Get_Trans()._41,
		m_pModelCom->Get_HierarchyNode("middle_01_r")->Get_Trans()._42,
		m_pModelCom->Get_HierarchyNode("middle_01_r")->Get_Trans()._43);
	ColliderDesc.vCenter = vCenter;
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"), TEXT("Com_OBB_Hand_R"), (CComponent**)&m_pColliderCom[COLLIDERTYPE_HAND_R], &ColliderDesc)))
		return E_FAIL;

	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));
	ColliderDesc.vSize = _float3(3.5f, 3.5f, 3.5f);
	vCenter = _float3(m_pModelCom->Get_HierarchyNode("middle_01_l")->Get_Trans()._41,
		m_pModelCom->Get_HierarchyNode("middle_01_l")->Get_Trans()._42,
		m_pModelCom->Get_HierarchyNode("middle_01_l")->Get_Trans()._43);
	ColliderDesc.vCenter = vCenter;
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"), TEXT("Com_OBB_Hand_L"), (CComponent**)&m_pColliderCom[COLLIDERTYPE_HAND_L], &ColliderDesc)))
		return E_FAIL;

	ZeroMemory(&ColliderDesc, sizeof(CCollider::COLLIDERDESC));
	ColliderDesc.vSize = _float3(3.7f, 3.8f, 3.7f);
	ColliderDesc.vCenter = _float3(0.f, 2.7f, 5.f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Collider_OBB"), TEXT("Com_OBB_Mouse"), (CComponent**)&m_pColliderCom[COLLIDERTYPE_MOUSE], &ColliderDesc)))
		return E_FAIL;

	/* For.Com_Navigation */
	CNavigation::NAVIGATIONDESC			NaviDesc;
	ZeroMemory(&NaviDesc, sizeof(CNavigation::NAVIGATIONDESC));
	NaviDesc.iCurrentIndex = 0;

	if (FAILED(__super::Add_Component(LEVEL_STAGE_02, TEXT("Prototype_Component_Navigation_Stage_02"), TEXT("Com_Navigation"), (CComponent**)&m_pNavigationCom, &NaviDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBoss_Bat::SetUp_ShaderResources()
{
	CGameInstance*		pGameInstance = GET_INSTANCE(CGameInstance);
	if (FAILED(m_pShaderCom->Set_RawValue("g_WorldMatrix", &m_pTransformCom->Get_WorldFloat4x4_TP(), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ViewMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_VIEW), sizeof(_float4x4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Set_RawValue("g_ProjMatrix", &pGameInstance->Get_TransformFloat4x4_TP(CPipeLine::D3DTS_PROJ), sizeof(_float4x4))))
		return E_FAIL;
	//if (FAILED(m_pShaderCom->Set_RawValue("g_vCamPosition", &pGameInstance->Get_CamPosition(), sizeof(_float4))))
	//	return E_FAIL;


	//const LIGHTDESC* pLightDesc = pGameInstance->Get_LightDesc(0);
	//if (nullptr == pLightDesc)
	//	return E_FAIL;

	//if (LIGHTDESC::TYPE_DIRECTIONAL == pLightDesc->eType)
	//{
	//	if (FAILED(m_pShaderCom->Set_RawValue("g_vLightDir", &pLightDesc->vDirection, sizeof(_float4))))
	//		return E_FAIL;
	//}

	//if (FAILED(m_pShaderCom->Set_RawValue("g_vLightDiffuse", &pLightDesc->vDiffuse, sizeof(_float4))))
	//	return E_FAIL;

	//if (FAILED(m_pShaderCom->Set_RawValue("g_vLightAmbient", &pLightDesc->vAmbient, sizeof(_float4))))
	//	return E_FAIL;

	///*if (FAILED(m_pShaderCom->Set_RawValue("g_vLightSpecular", &pLightDesc->vSpecular, sizeof(_float4))))
	//	return E_FAIL;*/

	RELEASE_INSTANCE(CGameInstance);

	return S_OK;
}

void CBoss_Bat::Ready_LimitTime()
{
	//BossBat_AttackL_01_1 - 185에 앞에 트레일
	m_vecLimitTime[BossBat_AttackL_01_1].push_back(120.f); // 왼손 on
	m_vecLimitTime[BossBat_AttackL_01_1].push_back(185.f); // 이펙트
	m_vecLimitTime[BossBat_AttackL_01_1].push_back(240.f); // 왼손 off

	//BossBat_AttackL_01_2b - 94에 뒤로 트레일
	m_vecLimitTime[BossBat_AttackL_01_2b].push_back(60.f); // 왼손 on
	m_vecLimitTime[BossBat_AttackL_01_2b].push_back(94.f); // 이펙트
	m_vecLimitTime[BossBat_AttackL_01_2b].push_back(230.f); // 왼손 off

	//BossBat_AttackL_01_3a - 94에 앞으로 트레일
	m_vecLimitTime[BossBat_AttackL_01_3a].push_back(70.f); // 오른손 on
	m_vecLimitTime[BossBat_AttackL_01_3a].push_back(94.f); // 이펙트
	m_vecLimitTime[BossBat_AttackL_01_3a].push_back(130.f); // 오른손 off

	//BossBat_AttackR_01_1 - 100에 앞으로 트레일
	m_vecLimitTime[BossBat_AttackR_01_1].push_back(40.f); // 오른손 on
	m_vecLimitTime[BossBat_AttackR_01_1].push_back(100.f); // 이펙트
	m_vecLimitTime[BossBat_AttackR_01_1].push_back(120.f); // 오른손 off

	//BossBat_AttackR_01_2b - 80에 뒤로 트레일
	m_vecLimitTime[BossBat_AttackR_01_2b].push_back(50.f); // 오른손 on
	m_vecLimitTime[BossBat_AttackR_01_2b].push_back(80.f); // 이펙트
	m_vecLimitTime[BossBat_AttackR_01_2b].push_back(230.f); // 오른손 off

	//BossBat_Bite_2 -> 여기부터 - 185 정도에 하나 캡쳐
	m_vecLimitTime[BossBat_Bite_2].push_back(120.f); // 입 on
	m_vecLimitTime[BossBat_Bite_2].push_back(185.f); // 이펙트
	m_vecLimitTime[BossBat_Bite_2].push_back(260.f); // 입 off

	//BossBat_Dash -> 여기부터
	m_vecLimitTime[BossBat_Dash].push_back(170.f); // 바디 무기로
	m_vecLimitTime[BossBat_Dash].push_back(400.f); // 바디 다시 피격으로

	//BossBat_HurtXL_R -> 여기부터
	m_vecLimitTime[BossBat_HurtXL_R].push_back(0.f); // 바디 무기로
	m_vecLimitTime[BossBat_HurtXL_R].push_back(10.f); // 바디 다시 피격으로

	//BossBat_HurtXL_L -> 여기부터
	m_vecLimitTime[BossBat_HurtXL_L].push_back(0.f); // 바디 무기로
	m_vecLimitTime[BossBat_HurtXL_L].push_back(10.f); // 바디 다시 피격으로

	//BossBat_JumpSmash_Chest  -> 여기부터
	m_vecLimitTime[BossBat_JumpSmash_Chest].push_back(20.f);
	m_vecLimitTime[BossBat_JumpSmash_Chest].push_back(60.f); // 바디 무기로
	m_vecLimitTime[BossBat_JumpSmash_Chest].push_back(120.f); // 바디 다시 피격으로

	//BossBat_JumpSmashForwardL  -> 여기부터
	m_vecLimitTime[BossBat_JumpSmashForwardL].push_back(60.f);
	m_vecLimitTime[BossBat_JumpSmashForwardL].push_back(150.f); // 바디 무기로
	m_vecLimitTime[BossBat_JumpSmashForwardL].push_back(220.f); // 바디 다시 피격으로
	m_vecLimitTime[BossBat_JumpSmashForwardL].push_back(400.f); // 아이들 상태로

	//BossBat_JumpSmashL  -> 여기부터
	m_vecLimitTime[BossBat_JumpSmashL].push_back(60.f); // y값 받아오기 시작
	m_vecLimitTime[BossBat_JumpSmashL].push_back(150.f); // 바디 무기로
	m_vecLimitTime[BossBat_JumpSmashL].push_back(220.f); // 바디 다시 피격으로
	m_vecLimitTime[BossBat_JumpSmashL].push_back(400.f); // 아이들 상태로

	m_vecLimitTime[BossBat_FTurn_R].push_back(0.f);
	m_vecLimitTime[BossBat_FTurn_R].push_back(100.f);

	m_vecLimitTime[BossBat_FTurn_L].push_back(0.f);
	m_vecLimitTime[BossBat_FTurn_L].push_back(100.f);
}

CBoss_Bat * CBoss_Bat::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CBoss_Bat*		pInstance = new CBoss_Bat(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CBoss_Bat"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CMonster * CBoss_Bat::Clone(void * pArg)
{
	CBoss_Bat*		pInstance = new CBoss_Bat(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CBoss_Bat"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBoss_Bat::Free()
{
	__super::Free();

	for (auto& _Collider : m_pColliderCom)
	{
		if (_Collider)
			Safe_Release(_Collider);
	}

	for (_uint i = 0; i < 2; ++i)
	{
		Safe_Release(m_pEyes[i]);
	}
}

void CBoss_Bat::Update_Collider()
{
	if (m_bCollision[COLLIDERTYPE_PUSH])
	{
		m_pColliderCom[COLLIDERTYPE_PUSH]->Update(m_pTransformCom->Get_WorldMatrix());
		CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_MONSTER_PUSH, m_pColliderCom[COLLIDERTYPE_PUSH], this);
	}

	if (m_bCollision[COLLIDERTYPE_BODY])
	{
		m_pColliderCom[COLLIDERTYPE_BODY]->Update(m_pTransformCom->Get_WorldMatrix());
		CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_MONSTER_BODY, m_pColliderCom[COLLIDERTYPE_BODY], this);
	}

	if (m_bCollision[COLLIDERTYPE_ATTBODY])
	{
		m_pColliderCom[COLLIDERTYPE_ATTBODY]->Update(m_pTransformCom->Get_WorldMatrix());
		CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_MONSTER_WEAPON, m_pColliderCom[COLLIDERTYPE_ATTBODY], this);
	}

	if (m_bCollision[COLLIDERTYPE_HAND_R])
	{
		m_pColliderCom[COLLIDERTYPE_HAND_R]->Update(m_pModelCom->Get_HierarchyNode("middle_01_r")->Get_CombinedTransformation()*XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())*m_pTransformCom->Get_WorldMatrix());
		CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_MONSTER_WEAPON, m_pColliderCom[COLLIDERTYPE_HAND_R], this);
	}

	if (m_bCollision[COLLIDERTYPE_HAND_L])
	{
		m_pColliderCom[COLLIDERTYPE_HAND_L]->Update(m_pModelCom->Get_HierarchyNode("middle_01_l")->Get_CombinedTransformation()*XMLoadFloat4x4(&m_pModelCom->Get_PivotMatrix())*m_pTransformCom->Get_WorldMatrix());
		CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_MONSTER_WEAPON, m_pColliderCom[COLLIDERTYPE_HAND_L], this);
	}

	if (m_bCollision[COLLIDERTYPE_MOUSE])
	{
		m_pColliderCom[COLLIDERTYPE_MOUSE]->Update(m_pTransformCom->Get_WorldMatrix());
		CCollisionMgr::Get_Instance()->Add_CollisoinList(CCollisionMgr::TYPE_MONSTER_WEAPON, m_pColliderCom[COLLIDERTYPE_MOUSE], this);
	}
}

void CBoss_Bat::Check_Stun()
{
	if (m_eMonsterState == CMonster::ATTACK_STUN)
	{
		//스턴하면 플레이어가 밀려남
		CPlayer* _pPlayer = static_cast<CPlayer*>(CGameInstance::Get_Instance()->Get_Player());
		_pPlayer->Set_AnimState(CPlayer::SD_HurtIdle);
		_pPlayer->Cancle();
		m_eMonsterState = CMonster::ATTACK_IDLE;
	}
}