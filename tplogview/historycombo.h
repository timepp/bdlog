#pragma once

class CHistoryComboBox : public ATL::CWindowImpl<CHistoryComboBox, CComboBox>
{
public:
	CHistoryComboBox(std::wstring& history) : m_history(history)
	{
	}

	BEGIN_MSG_MAP(CHistoryComboBox)
	END_MSG_MAP()

	void LoadHistory()
	{
		for (size_t a = 0, b = 0; a < m_history.length();)
		{
			b = m_history.find(L'\t', a);
			if (b == std::wstring::npos) b = m_history.length();
			if (b > a)
			{
				AddString(m_history.substr(a, b-a).c_str());
			}
			a = b+1;
		}
	}

	void SaveHistory()
	{
		CStringW text;
		GetWindowText(text);

		int lbpos = -1;
		std::wstring history;
		for (int i = 0; i < GetCount(); i++)
		{
			CStringW lbtext;
			GetLBText(i, lbtext);
			if (lbtext == text)
			{
				lbpos = i;
			}
			else if (i < 32)
			{
				history += lbtext;
				history += L'\t';
			}
		}

		if (lbpos > 0)
		{
			DeleteString((UINT)lbpos);
		}

		InsertString(0, text);
		m_history = std::wstring(text) + L"\t" + history;
	}

private:
	std::wstring& m_history;
};
