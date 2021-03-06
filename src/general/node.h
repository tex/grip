#ifndef __NODE_H__
#define __NODE_H__

#include <memory>
#include <list>
#include <string>
#include <climits>
#include "dbreader.h"
#include "ids.h"


class Node;
typedef std::shared_ptr<Node> NodePtr;


class Node
{
	public:
		enum Value
		{
			NODE_EMPTY = UCHAR_MAX + 1,
			NODE_SPLIT,
			NODE_END,
			NODE_MARK = UINT_MAX - (UINT_MAX / 2),
		};

	public:
		Node(int val = NODE_EMPTY);

		void parseFixedString(const std::string &exp, bool caseSensitive);
		void parseRegex(const std::string &exp, bool extended, bool caseSensitive);

		bool isUnambiguous(unsigned charsNo = 0) const;
		void findIds(Ids &res, DbReader &db) const;

		const std::list<NodePtr> &getNext() const;
		int getVal() const;

		/* debug methods */
		std::string toString(bool unique=false) const;
		void makeDotGraph(std::string &graph);

	private:
		void tokenizeFixedString(const char *str);
		Node *tokenizeRegex(const unsigned char **exp, bool extended, bool nested);

		void makeDotGraphMarked(std::string &graph);

		void markAll();
		void markAlpha();
		void permuteCaseMarked();

		void findIds(Ids &res, const Ids &ids, DbReader &db, uint32_t trigram) const;

		Node *addNext(int val = NODE_EMPTY);
		Node *addCommonDescendant(NodePtr desc);

	private:
		unsigned val;
		std::list<NodePtr> next;
};

#endif
