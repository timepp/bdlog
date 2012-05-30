#ifndef BD_SECURITY_H
#define BD_SECURITY_H

#include <windows.h>
#pragma warning(push)
#pragma warning(disable:4668)
#include <aclapi.h>
#pragma warning(pop)
#include <Sddl.h>

class CLowestRightsSecurityAttribute3
{
public:
	CLowestRightsSecurityAttribute3() : m_pSD(NULL), m_bValid(FALSE)
	{
		ZeroMemory(&m_SA, sizeof(m_SA));
		m_SA.nLength = sizeof(m_SA);
		m_pSD = (PSECURITY_DESCRIPTOR) HeapAlloc( GetProcessHeap(),
			HEAP_ZERO_MEMORY,
			SECURITY_DESCRIPTOR_MIN_LENGTH);

		/*
		* Create a security descriptor that will allow
		* everyone full access.
		*/
		InitializeSecurityDescriptor( m_pSD, SECURITY_DESCRIPTOR_REVISION );

		// DACL

#pragma warning(push)
#pragma warning(disable: 6248)
		SetSecurityDescriptorDacl( m_pSD, TRUE, NULL, FALSE );
#pragma warning(pop)

		// SACL
		{
			PSECURITY_DESCRIPTOR pSD = NULL;  

			PACL pSacl = NULL; // not allocated
			BOOL fSaclPresent = FALSE;
			BOOL fSaclDefaulted = FALSE;

			if (ConvertStringSecurityDescriptorToSecurityDescriptorW(
				L"S:(ML;;;;;LW)", SDDL_REVISION_1, &pSD, NULL)) 
			{
				if (GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl, 
					&fSaclDefaulted))
				{
					SetSecurityDescriptorSacl(m_pSD, TRUE, pSacl, FALSE);
				}
			}
		}


		m_SA.bInheritHandle = TRUE;
		m_SA.lpSecurityDescriptor = m_pSD;
	}
	~CLowestRightsSecurityAttribute3()
	{
		if( m_pSD ) HeapFree( GetProcessHeap(), 0, m_pSD );
	}

	operator LPSECURITY_ATTRIBUTES()
	{
		return &m_SA;
	}

private:
	BOOL m_bValid;
	SECURITY_ATTRIBUTES m_SA;
	PSECURITY_DESCRIPTOR m_pSD;
};


typedef CLowestRightsSecurityAttribute3 CLowestRightsSecurityAttribute;

#endif
