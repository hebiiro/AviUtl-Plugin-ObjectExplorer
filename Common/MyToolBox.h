#pragma once

#define COM_TRY(expression) \
 \
do \
{ \
	HRESULT hr = (expression); \
 \
	if (FAILED(hr)) \
	{ \
		MY_TRACE(_T("failed : %s\n"), _T(#expression)); \
 \
		return hr; \
	} \
} \
while (0)

inline HRESULT WINAPI saveXMLDocument(
	const MSXML2::IXMLDOMDocumentPtr& document,
	LPCWSTR path, _bstr_t encoding = L"UTF-16LE")
{
	IStreamPtr stream;
	COM_TRY(::SHCreateStreamOnFileW(
		path,
		STGM_WRITE | STGM_SHARE_DENY_WRITE |
		STGM_CREATE | STGM_DIRECT,
		&stream));

	MSXML2::IMXWriterPtr writer(__uuidof(MSXML2::MXXMLWriter));
	writer->indent = VARIANT_TRUE;
	writer->byteOrderMark = VARIANT_TRUE;
//	writer->omitXMLDeclaration = VARIANT_TRUE;
//	writer->version = L"1.0";
//	writer->encoding = encoding;
	writer->output = stream.GetInterfacePtr();

	MSXML2::ISAXXMLReaderPtr reader(__uuidof(MSXML2::SAXXMLReader));
	reader->putContentHandler(MSXML2::ISAXContentHandlerPtr(writer));

	return reader->parse(document.GetInterfacePtr());
}

inline MSXML2::IXMLDOMElementPtr appendElement(
	const MSXML2::IXMLDOMDocumentPtr& document, const MSXML2::IXMLDOMNodePtr& node, BSTR tag)
{
	MSXML2::IXMLDOMElementPtr newElement = document->createElement(tag);
	return node->appendChild(newElement);
}

inline MSXML2::IXMLDOMElementPtr appendElement(
	const MSXML2::IXMLDOMNodePtr& node, BSTR tag)
{
	return appendElement(node->ownerDocument, node, tag);
}

inline MSXML2::IXMLDOMTextPtr appendText(
	const MSXML2::IXMLDOMDocumentPtr& document, const MSXML2::IXMLDOMNodePtr& node, BSTR text)
{
	MSXML2::IXMLDOMTextPtr newText = document->createTextNode(text);
	return node->appendChild(newText);
}

inline MSXML2::IXMLDOMTextPtr appendText(
	const MSXML2::IXMLDOMNodePtr& node, BSTR text)
{
	return appendText(node->ownerDocument, node, text);
}

inline HRESULT WINAPI GetPrivateProfileVARIANT(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, _variant_t& outValue)
{
	outValue = element->getAttribute(name);

	return S_OK;
}

inline HRESULT WINAPI GetPrivateProfileBSTR(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, _bstr_t& outValue)
{
	try
	{
		_variant_t var = element->getAttribute(name);

		if (var.vt == VT_NULL)
			return S_FALSE;

		outValue = var;

		return S_OK;
	}
	catch (_com_error& e)
	{
		return e.Error();
	}
}

template<class T>
inline HRESULT WINAPI GetPrivateProfileInt(
	_variant_t& var, T& outValue, int radix = 0)
{
	try
	{
		if (var.vt == VT_NULL)
			return S_FALSE;

		_bstr_t value = var;

		if (!(BSTR)value)
			return S_FALSE;

		outValue = (T)wcstol(value, 0, radix);

		return S_OK;
	}
	catch (_com_error& e)
	{
		return e.Error();
	}
}

template<class T>
inline HRESULT WINAPI GetPrivateProfileInt(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, T& outValue, int radix = 0)
{
	return GetPrivateProfileInt(element->getAttribute(name), outValue, radix);
}

template<class T>
inline HRESULT WINAPI GetPrivateProfileFloat(variant_t& var, T& outValue)
{
	try
	{
		if (var.vt == VT_NULL)
			return S_FALSE;

		_bstr_t value = var;

		if (!(BSTR)value)
			return S_FALSE;

		outValue = (T)wcstod(value, 0);

		return S_OK;
	}
	catch (_com_error& e)
	{
		return e.Error();
	}
}

template<class T>
inline HRESULT WINAPI GetPrivateProfileFloat(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, T& outValue)
{
	return GetPrivateProfileFloat(element->getAttribute(name), outValue);
}

template<class T>
inline HRESULT WINAPI GetPrivateProfileString(_variant_t& var, T& outValue)
{
	try
	{
		if (var.vt == VT_NULL)
			return S_FALSE;

		_bstr_t value = var;

		if (!(BSTR)value)
			return S_FALSE;

		outValue = (BSTR)value;

		return S_OK;
	}
	catch (_com_error& e)
	{
		return e.Error();
	}
}

template<class T>
inline HRESULT WINAPI GetPrivateProfileString(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, T& outValue)
{
	return GetPrivateProfileString(element->getAttribute(name), outValue);
}

template<class T>
inline HRESULT WINAPI GetPrivateProfileBool(_variant_t& var, T& outValue)
{
	try
	{
		if (var.vt == VT_NULL)
			return S_FALSE;

		_bstr_t value = var;

		if (!(BSTR)value || ::lstrlenW(value) == 0)
			return S_FALSE;

		if (::StrCmpIW(value, L"ON") == 0)
		{
			outValue = true;
		}
		else if (::StrCmpIW(value, L"OFF") == 0)
		{
			outValue = false;
		}
		else if (::StrCmpIW(value, L"YES") == 0)
		{
			outValue = true;
		}
		else if (::StrCmpIW(value, L"NO") == 0)
		{
			outValue = false;
		}
		else
		{
			outValue = !!wcstol(value, 0, 0);
		}

		return S_OK;
	}
	catch (_com_error& e)
	{
		return e.Error();
	}
}

template<class T>
inline HRESULT WINAPI GetPrivateProfileBool(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, T& outValue)
{
	return GetPrivateProfileBool(element->getAttribute(name), outValue);
}

inline HRESULT WINAPI GetPrivateProfileColor(_variant_t& var, COLORREF& outValue)
{
	try
	{
		if (var.vt == VT_NULL)
			return S_FALSE;

		_bstr_t value = var;

		if (!(BSTR)value)
			return S_FALSE;

		int c = ::lstrlenW(value);

		if (c == 0)
			return S_FALSE;

		BSTR bstrValue = value;

		if (*bstrValue == L'#')
		{
			DWORD temp = wcstoul(bstrValue + 1, 0, 16);

			if (c == 4)
			{
				// #fc8

				DWORD r = (temp & 0x0F00) >> 8;
				DWORD g = (temp & 0x00F0) >> 4;
				DWORD b = (temp & 0x000F) >> 0;

				r |= r << 4;
				g |= g << 4;
				b |= b << 4;

				outValue = RGB(r, g, b);
			}
			else
			{
				// #ffcc88

				DWORD r = (temp & 0x00FF0000) >> 16;
				DWORD g = (temp & 0x0000FF00) >> 8;
				DWORD b = (temp & 0x000000FF) >> 0;

				outValue = RGB(r, g, b);
			}
		}
		else
		{
			BSTR sep1 = ::StrChrW(bstrValue, L',');

			if (!sep1)
				return S_FALSE;

			BSTR sep2 = ::StrChrW(sep1 + 1, L',');

			if (!sep2)
				return S_FALSE;

			*sep1 = L'\0';
			*sep2 = L'\0';

			DWORD r = wcstoul(bstrValue, 0, 0);
			DWORD g = wcstoul(sep1 + 1, 0, 0);
			DWORD b = wcstoul(sep2 + 1, 0, 0);

			outValue = RGB(r, g, b);
		}

		return S_OK;
	}
	catch (_com_error& e)
	{
		return e.Error();
	}
}

inline HRESULT WINAPI GetPrivateProfileColor(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, COLORREF& outValue)
{
	return GetPrivateProfileColor(element->getAttribute(name), outValue);
}

inline HRESULT WINAPI GetPrivateProfileColor(_variant_t& var, Gdiplus::Color& outValue)
{
	try
	{
		if (var.vt == VT_NULL)
			return S_FALSE;

		_bstr_t value = var;

		if (!(BSTR)value)
			return S_FALSE;

		int c = ::lstrlenW(value);

		if (c == 0)
			return S_FALSE;

		BSTR bstrValue = value;

		if (*bstrValue == L'#')
		{
			DWORD temp = wcstoul(bstrValue + 1, 0, 16);

			if (c == 4)
			{
				// #fc8

				DWORD r = (temp & 0x0F00) >> 8;
				DWORD g = (temp & 0x00F0) >> 4;
				DWORD b = (temp & 0x000F) >> 0;

				r |= r << 4;
				g |= g << 4;
				b |= b << 4;

				outValue.SetFromCOLORREF(RGB(r, g, b));
			}
			else
			{
				// #ffcc88

				DWORD r = (temp & 0x00FF0000) >> 16;
				DWORD g = (temp & 0x0000FF00) >> 8;
				DWORD b = (temp & 0x000000FF) >> 0;

				outValue.SetFromCOLORREF(RGB(r, g, b));
			}
		}
		else
		{
			BSTR sep1 = ::StrChrW(bstrValue, L',');

			if (!sep1)
			{
				outValue.SetValue(wcstoul(bstrValue, 0, 16));
			}
			else
			{
				BSTR sep2 = ::StrChrW(sep1 + 1, L',');

				if (!sep2)
					return S_FALSE;

				BSTR sep3 = ::StrChrW(sep2 + 1, L',');

				if (!sep3)
				{
					*sep1 = L'\0';
					*sep2 = L'\0';

					DWORD r = wcstoul(bstrValue, 0, 0);
					DWORD g = wcstoul(sep1 + 1, 0, 0);
					DWORD b = wcstoul(sep2 + 1, 0, 0);

					outValue.SetFromCOLORREF(RGB(r, g, b));
				}
				else
				{
					*sep1 = L'\0';
					*sep2 = L'\0';
					*sep3 = L'\0';

					BYTE a = (BYTE)wcstoul(bstrValue, 0, 0);
					BYTE r = (BYTE)wcstoul(sep1 + 1, 0, 0);
					BYTE g = (BYTE)wcstoul(sep2 + 1, 0, 0);
					BYTE b = (BYTE)wcstoul(sep3 + 1, 0, 0);

					outValue.SetValue(outValue.MakeARGB(a, r, g, b));
				}
			}
		}

		return S_OK;
	}
	catch (_com_error& e)
	{
		return e.Error();
	}
}

inline HRESULT WINAPI GetPrivateProfileColor(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, Gdiplus::Color& outValue)
{
	return GetPrivateProfileColor(element->getAttribute(name), outValue);
}

template<class T, class S>
inline HRESULT WINAPI GetPrivateProfilePercent(_variant_t& var, T& outValue, S& outValuePercent)
{
	try
	{
		if (var.vt == VT_NULL)
			return S_FALSE;

		_bstr_t value = var;

		if (!(BSTR)value || ::lstrlenW(value) == 0)
			return S_FALSE;

		if (::StrChrW(value, L'%'))
			outValuePercent = wcstol(value, 0, 0);
		else
			outValue = wcstol(value, 0, 0);

		return S_OK;
	}
	catch (_com_error& e)
	{
		return e.Error();
	}
}

template<class T, class S>
inline HRESULT WINAPI GetPrivateProfilePercent(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, T& outValue, S& outValuePercent)
{
	return GetPrivateProfilePercent(element->getAttribute(name), outValue, outValuePercent);
}

template<class T>
inline HRESULT WINAPI GetPrivateProfileTime(_variant_t& var, T& outValue)
{
	try
	{
		if (var.vt == VT_NULL)
			return S_FALSE;

		_bstr_t value = var;

		if (!(BSTR)value || ::lstrlenW(value) == 0)
			return S_FALSE;

		LPWSTR sep = ::StrChrW(value, L':');

		if (sep)
		{
			*sep = L'\0';
			outValue = _wtof(value) * 60 + _wtof(sep + 1);
		}
		else
		{
			outValue = _wtof(value);
		}

		return S_OK;
	}
	catch (_com_error& e)
	{
		return e.Error();
	}
}

template<class T>
inline HRESULT WINAPI GetPrivateProfileTime(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, T& outValue)
{
	return GetPrivateProfileTime(element->getAttribute(name), outValue);
}

template<class T, class A>
inline HRESULT WINAPI GetPrivateProfileLabel(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, T& outValue, const A& array)
{
	_bstr_t value;

	HRESULT hr = GetPrivateProfileBSTR(element, name, value);

	if (hr != S_OK)
		return hr;

	int c = _countof(array);

	for (int i = 0; i < c; i++)
	{
		if (::lstrcmpW(array[i].label, value) == 0)
		{
			MY_TRACE_WSTR(array[i].label);
			outValue = array[i].value;
			return S_OK;
		}
	}

	return S_FALSE;
}

template<class T>
inline HRESULT WINAPI SetPrivateProfileString(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, const T& value)
{
	return element->setAttribute(name, value);
}

template<class T>
inline HRESULT WINAPI SetPrivateProfileInt(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, const T& value)
{
	return element->setAttribute(name, value);
}

template<class T>
inline HRESULT WINAPI SetPrivateProfileFloat(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, const T& value)
{
	return element->setAttribute(name, value);
}

inline HRESULT WINAPI SetPrivateProfileColor(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, const Gdiplus::Color& value)
{
	WCHAR text[MAX_PATH] = {};
	::StringCbPrintfW(text, sizeof(text), L"%08X", value.GetValue());

	return element->setAttribute(name, text);
}

template<class T, class A>
inline HRESULT WINAPI SetPrivateProfileLabel(
	const MSXML2::IXMLDOMElementPtr& element, BSTR name, const T& value, const A& array)
{
	int c = _countof(array);

	for (int i = 0; i < c; i++)
	{
		if (array[i].value == value)
			return element->setAttribute(name, array[i].label);
	}

	return S_FALSE;
}
