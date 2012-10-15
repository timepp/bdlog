#pragma once

#include <string.h>

struct lstr
{
	lstr(const wchar_t*s, size_t l) : str(s), len(l)
	{
	}
	const wchar_t* str;
	size_t len;
};

#define LSTR(s) lstr(s, _countof(s)-1)

class textstream
{
private:
	wchar_t* m_buffer;
	wchar_t* m_p;
	size_t m_avail;
public:
	textstream(wchar_t* buffer, size_t len)
		: m_buffer(buffer), m_avail(len), m_p(buffer)
	{
	}

	operator wchar_t* ()
	{
		return m_buffer;
	}

	textstream& operator <<(const wchar_t* str)
	{
		store(str, wcslen(str));
		return *this;
	}
	textstream& operator <<(lstr str)
	{
		store(str.str, str.len);
		return *this;
	}
	textstream& operator <<(wchar_t ch)
	{
		if (m_avail > 1)
		{
			*m_p = ch;
			advance(1);
		}
		return *this;
	}

	void advance(size_t len)
	{
		if (len >= m_avail) len = m_avail - 1;
		m_p += len;
		m_avail -= len;
		*m_p = L'\0';
	}

	size_t len() const
	{
		return static_cast<size_t>(m_p - m_buffer);
	}

private:
	void store(const wchar_t* str, size_t len)
	{
		if (m_avail <= 1) return;
		if (len >= m_avail) len = m_avail - 1;
		memcpy(m_p, str, len * 2);
		advance(len);
	}
};

class binarystream
{
public:
	binarystream() : m_buffer(m_static_buffer), m_capacity(sizeof(m_static_buffer)), m_len(0)
	{
	}
	~binarystream()
	{
		if (m_buffer != m_static_buffer)
			delete [] m_buffer;
	}

	void clear() {	m_len = 0; }
	size_t len() { return m_len; }

	operator const void* () { return m_buffer; }

	void store_buffer(const void* buffer, size_t len)
	{
		size_t cap = m_capacity;
		while (m_len + len > cap)
		{
			cap *= 2;
		}
		enlarge(cap);

		memcpy(m_buffer + m_len, buffer, len);
		m_len += len;
	}

	void store_str(const wchar_t* str, size_t len = 0)
	{
		if (len == 0) len = wcslen(str);
		len *= 2;
		store_buffer(str, len);
	}

	void store_str_prefix_len(const wchar_t* str, size_t len = 0)
	{
		if (len == 0) len = wcslen(str);
		len *= 2;
		store(static_cast<unsigned __int32>(len));
		store_buffer(str, len);
	}

	template <typename T>
	void store(const T& val)
	{
		store_buffer(&val, sizeof(val));
	}

private:
	char* m_buffer;
	char m_static_buffer[256];
	size_t m_capacity;
	size_t m_len;
	size_t m_read_pos;

	void enlarge(size_t cap)
	{
		if (m_capacity >= cap) return;

		char* buffer = new char[cap];
		memcpy(buffer, m_buffer, m_len);
		if (m_buffer != m_static_buffer)
		{
			delete [] m_buffer;
		}
		m_buffer = buffer;
		m_capacity = cap;
	}
};

#define TRUNCATED_COPY(d, s) textstream(d, _countof(d)) << s
