#pragma once
#include "Base.h"
class CStageMgr :
	public CBase
{
	DECLARE_SINGLETON(CStageMgr)
private:
	CStageMgr();
	virtual ~CStageMgr() = default;

public:
	const _uint& Add_Mob() { return ++m_iMob; }

private:
	_uint m_iMob = 0;

public:
	// CBase을(를) 통해 상속됨
	virtual void Free() override;

};

