#include "glob.h"
#include "dir.h"
#include "error.h"

#if !defined(_POSIX_C_SOURCE)
#include "external/fnmatch.c"
#endif


using namespace std;


Glob::Glob() : m_flags(0)
{}

void Glob::addExcludePattern(const string &pattern)
{
	m_excludes.push_back(pattern);
}

void Glob::addIncludePattern(const string &pattern)
{
	m_includes.push_back(pattern);
}

void Glob::caseSensitive(bool enable)
{
#if defined(FNM_CASEFOLD)
	// only available with GNU fnmatch extension
	if (enable)
		m_flags &= ~FNM_CASEFOLD;
	else
		m_flags |= FNM_CASEFOLD;
#endif
}

void Glob::extendedMatch(bool enable)
{
#if defined(FNM_EXTMATCH)
	// only available with GNU fnmatch extension
	if (enable)
		m_flags |= FNM_EXTMATCH;
	else
		m_flags &= ~FNM_EXTMATCH;
#else
	(void) enable;
	throw ThisError("extended globbing not supported");
#endif
}

bool Glob::compare(const string &str) const
{
	size_t pos = str.rfind(PATH_DELIMITER);
	const char *fname = str.c_str() + (pos == string::npos ? 0 : pos + 1);

	for (const string &pattern : m_excludes)
	{
		int res = fnmatch(pattern.c_str(), fname, m_flags);
		if (res == 0)
			return false;
		else if (res != FNM_NOMATCH)
			throw ThisError("invalid glob pattern", res);
	}

	if (m_includes.empty())
		return true;

	for (const string &pattern : m_includes)
	{
		int res = fnmatch(pattern.c_str(), fname, m_flags);
		if (res == 0)
			return true;
		else if (res != FNM_NOMATCH)
			throw ThisError("invalid glob pattern", res);
	}

	return false;
}