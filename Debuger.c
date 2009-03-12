
/* -- $Id: Debuger.cpp,v 1.12 2004/04/30 03:20:20 qianjinqi Exp $ -- */  
	
#include "Debuger.h"
	CDebuger * CDebuger::pDebuger = NULL;
CDebuger::CDebuger () 
{
	m_debug_level = 0;
}

CDebuger::~CDebuger () 
{
}

CDebuger * CDebuger::Instance () 
{
	if (pDebuger == NULL)
		
	{
		pDebuger = new CDebuger ();
	}
	return pDebuger;
}
void 
CDebuger::SetDebugLevel (int debug_level) 
{
	m_debug_level = debug_level;
} int 

CDebuger::GetDebugLevel () 
{
	return m_debug_level;
}


