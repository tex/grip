#include "pattern.h"
#include "case.h"
#include "error.h"
#include <sys/types.h>
#include <cstring>

#if defined(USE_BOOST)
#include <boost/regex.h>
#elif defined(_POSIX_C_SOURCE)
#include <regex.h>
#endif

using namespace std;


class LiteralPattern : public Pattern
{
	public:
		LiteralPattern(const string &pattern) :
			m_pattern(pattern)
		{}

		virtual ~LiteralPattern()
		{}

		virtual void tokenize(Node &tree) const
		{
			tree.parseFixedString(m_pattern, true);
		}

		virtual Match match(const char *str, bool wholeLine) const
		{
			(void)wholeLine;
			Match res(strstr(str, m_pattern.c_str()));

			if (res.pos)
				res.len = m_pattern.size();

			return res;
		}

	private:
		string m_pattern;
};


class LiteralCaseInsPattern : public Pattern
{
	public:
		LiteralCaseInsPattern(const string &pattern) :
			m_pattern(pattern)
		{
			for (size_t i = 0; i < pattern.size(); i++)
				m_pattern[i] = TO_LOWER(m_pattern[i]);
		}

		virtual ~LiteralCaseInsPattern()
		{}

		virtual void tokenize(Node &tree) const
		{
			tree.parseFixedString(m_pattern, false);
		}

		virtual Match match(const char *str, bool wholeLine) const
		{
			(void)wholeLine;
			const char *p = m_pattern.c_str();

			for (const char *ch = str; *ch != '\0'; ch++)
			{
				if (TO_LOWER(*ch) == *p)
				{
					if (*++p == '\0')
						return Match(ch-m_pattern.size()+1, m_pattern.size());
				}
				else
				{
					p = m_pattern.c_str();
				}
			}

			return Match();
		}

	private:
		string m_pattern;
};


class RegexPattern : public Pattern
{
	public:
		RegexPattern(const string &pattern, bool extended, bool cs) :
			m_pattern(pattern),
			m_extended(extended),
			m_caseSensitive(cs)
		{
			int flags = 0;
			flags |= extended ? REG_EXTENDED : 0;
			flags |= !cs ? REG_ICASE : 0;

			int res = regcomp(&m_regex, pattern.c_str(), flags);
			if (res != 0)
			{
				throw ThisError("malformed regular expression")
					.add("regex", pattern)
					.add("msg", getError(res));
			}
		}

		virtual ~RegexPattern()
		{
			regfree(&m_regex);
		}

		virtual void tokenize(Node &tree) const
		{
			tree.parseRegex(m_pattern, m_extended, m_caseSensitive);
		}

		virtual Match match(const char *str, bool wholeLine) const
		{
			regmatch_t res;
			int flags = wholeLine ? 0 : REG_NOTBOL;
			int code = regexec(&m_regex, str, 1, &res, flags);

			if (code == 0)
			{
				return Match(str + res.rm_so, res.rm_eo - res.rm_so);
			}
			else if (code == REG_NOMATCH)
			{
				return Match();
			}
			else
			{
				throw ThisError("malformed regular expression")
					.add("regex", m_pattern)
					.add("msg", getError(code));
			}
		}

	private:
		string getError(int code) const
		{
			char buf[1024];
			regerror(code, &m_regex, buf, sizeof(buf));
			return buf;
		}

	private:
		string m_pattern;
		regex_t m_regex;
		bool m_extended;
		bool m_caseSensitive;
};


/*** Pattern ***/

Pattern::~Pattern()
{}

Pattern *Pattern::create(const string &pattern, Mode mode, bool caseSensitive)
{
	if (mode == FIXED)
	{
		if (caseSensitive)
			return new LiteralPattern(pattern);
		else
			return new LiteralCaseInsPattern(pattern);
	}
	else
	{
		bool extended = (mode == EXTENDED);
		return new RegexPattern(pattern, extended, caseSensitive);
	}
}

Pattern::Match::Match(const char *pos, size_t len) : pos(pos), len(len)
{}
