#include "Query.h"
#include "TextQuery.h"
#include <memory>
#include <set>
#include <algorithm>
#include <iostream>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <regex>

using namespace std;

template <class Container>
void split(const std::string& str, Container& cont, char delim = ' ')
{
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) 
    {
        cont.push_back(token);
    }
}

shared_ptr<QueryBase> QueryBase::factory(const string& s)
{
    vector<string> vec;
    split(s,vec ,' ');

    if(vec.size() == 1)
    {
      return std::shared_ptr<QueryBase>(new WordQuery(s));
    }

    else if (vec.size() == 3)
    {
        if(vec.at(0)=="AND")
            return std::shared_ptr<QueryBase>(new AndQuery(vec.at(1),vec.at(2)));
        else if(vec.at(0)=="OR")
            return std::shared_ptr<QueryBase>(new OrQuery(vec.at(1),vec.at(2)));
        else if(vec.at(0)=="AD")
            return std::shared_ptr<QueryBase>(new AdjacentQuery(vec.at(1),vec.at(2)));
        else
        throw std::invalid_argument("Unrecognized search");
    }
 
    else 
    {
      throw std::invalid_argument("Unrecognized search");
    }
}


QueryResult AndQuery::eval (const TextQuery& text) const
{
    QueryResult left_result = text.query(left_query);
    QueryResult right_result = text.query(right_query);
    auto ret_lines = make_shared<set<line_no>>();
    set_intersection(left_result.begin(), left_result.end(),
        right_result.begin(), right_result.end(), 
        inserter(*ret_lines, ret_lines->begin()));
   return QueryResult(rep(), ret_lines, left_result.get_file());
}

QueryResult OrQuery::eval(const TextQuery &text) const
{
    QueryResult left_result = text.query(left_query);
    QueryResult right_result = text.query(right_query);
    auto ret_lines = make_shared<set<line_no>>(left_result.begin(), left_result.end());
    ret_lines->insert(right_result.begin(), right_result.end());
    return QueryResult(rep(), ret_lines, left_result.get_file());
}

QueryResult AdjacentQuery::eval (const TextQuery& text) const
{
    QueryResult left_result = text.query(left_query);
    QueryResult right_result = text.query(right_query);
    auto beg = left_result.begin(), end = left_result.end();
	auto sz = left_result.get_file()->size();

    auto ret_lines = std::make_shared<std::set<line_no>>();
    auto ret_lines2 = std::make_shared<std::set<line_no>>();
    auto ret_lines3 = std::make_shared<std::set<line_no>>();

    ret_lines->insert(left_result.begin(), left_result.end());
    ret_lines2->insert(right_result.begin(), right_result.end());

    int count = 0;

    for (auto i = ret_lines->begin(); i != ret_lines->end(); ++i)
    {
        for(auto j = ret_lines2->begin(); j != ret_lines2->end(); ++j){
            if(i != prev(ret_lines->end()) && left_result.get_file().get()->at(*i+1) == left_result.get_file().get()->at(*j))
            {
                ret_lines3->insert(*i);
                ret_lines3->insert(*j);
                count++;
            }
            else if(i != ret_lines->begin() && left_result.get_file().get()->at(*i-1) == left_result.get_file().get()->at(*j))
            {
                ret_lines3->insert(*i);
                ret_lines3->insert(*j);
                count++;
            }
            else if(j != ret_lines->begin() && left_result.get_file().get()->at(*j-1) == left_result.get_file().get()->at(*i))
            {
                ret_lines3->insert(*i);
                ret_lines3->insert(*j);
                count++;
            }
            else if(j != prev(ret_lines->end()) && left_result.get_file().get()->at(*j+1) == left_result.get_file().get()->at(*i))
            {
                ret_lines3->insert(*i);
                ret_lines3->insert(*j);
                count++;
            }
        }
    }

    return QueryResult(rep(), ret_lines3, left_result.get_file());
}

std::ostream &print(std::ostream &os, const QueryResult &qr)
{
    os << "\"" << qr.sought << "\"" << " occurs " << 
        qr.lines->size() << " times:" <<std::endl;
    for (auto num : *qr.lines)
    {
        os << "\t(line " << num + 1 << ") " 
            << *(qr.file->begin() + num) << std::endl;
    }
    return os;
}
