/**
 * @file   LuckyGoal.cpp
 * @author Jiangwen Su <lastz.cn@gmail.com>
 * @date   Sat Jul 25 19:04:53 2009
 * 
 * @brief  
 * 
 * 
 */

#include <stdlib.h>

#include <urlget.h>
#include <Encoding.h>

#include <iostream>
#include <iomanip>

#include <bzlib.h>

#include <boost/regex.hpp>

#include <ucommon/fsys.h>
#include <sys/stat.h>
#include <ucommon/thread.h>
using namespace ucc;

using namespace lastz;

std::string GetHtmlByUrl(const std::string& url){
    std::string strHtml;
    
    UrlGet ug;
    if ( !ug.GetUrl(url) ){
        std::cout << "==!==Open URL: " << std::string(url) << "failed. ==!==" << std::endl;
    }else{
        std::string strHtmlGB = ug.GetDoc();
        strHtml = Encoding::GBToUTF8(strHtmlGB);

        //std::cout << strHtml << std::endl;
        std::cout << "==-== Get URL OK. ==-==" << url << std::endl;
    }

    return strHtml;
}

std::string GetGBHtmlByUrl(const std::string& url){
    std::string strHtml;
    
    UrlGet ug;
    if ( !ug.GetUrl(url) ){
        std::cout << "==!==Open URL: " << std::string(url) << "failed. ==!==" << std::endl;
    }else{
        std::string strHtml = ug.GetDoc();
        //std::cout << strHtml << std::endl;
        std::cout << "==-== Get URL OK. ==-==" << url << std::endl;
    }

    return strHtml;
}

bool CreateDirs(const std::string& strDirs)
{
    bool bOK = true;

    size_t start = 0;
    size_t pos = strDirs.find("/", start + 1);
    while ( pos != std::string::npos ){
        std::string subdir = strDirs.substr(0, pos);
        if ( !fsys::isdir(subdir.c_str()) ){
            fsys::createDir(subdir.c_str(), S_IRWXU | S_IRGRP | S_IXGRP);
        }
        start = pos;
        pos = strDirs.find("/", start + 1);        
    }
    
    return bOK;
}

bool SaveFile(const std::string& strFullFileName, const std::string& strText, bool bZip)
{
    unsigned int nTextLen = strText.length();
    if ( nTextLen == 0 ) return true;
    
    CreateDirs(strFullFileName);

    std::string filename = strFullFileName;
    

    const char* buf = strText.c_str();
    char* destBuf = NULL;
    
    if ( bZip ){
        filename = filename + ".bz2";
        
        destBuf = new char[nTextLen * 2];
        unsigned int nZippedLen = nTextLen * 2;
        char* srcBuf = new char[nTextLen];
        memcpy(srcBuf, strText.c_str(), nTextLen);
            
        int nRet = BZ2_bzBuffToBuffCompress(destBuf, &nZippedLen, srcBuf, nTextLen, 9, 0, 0);
        delete srcBuf;
        srcBuf = NULL;
        
        if ( nRet == BZ_OK && nZippedLen > 0 ){
            nTextLen = nZippedLen;
            buf = destBuf;
        } else{
            std::cout << "!!!!!!!! BZ2_bzBuffToBuffCompress " << filename << " error. code = " << nRet << std::endl;
            return false;
        }
    } 
    
    bool bOK = false;
    std::fstream of;
    of.open(filename.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    if ( of.is_open() ){
        of.write( buf, nTextLen);
        of.close();
        bOK = true;
    }

    if ( destBuf != NULL ){
        delete destBuf;
    }
    
    return bOK;
}

bool LoadFile(const std::string& strFullFileName, std::string& strText, bool bZip)
{
    bool bOK = false;

    std::string filename = strFullFileName;
    if ( bZip ){
        filename = filename + ".bz2";
    }
    
    std::fstream is;
    is.open(filename.c_str(), std::ios::in | std::ios::binary);
    if ( is.is_open() ){
        std::cout << "file: " << filename << " opened." << std::endl;
        
        is.seekg(0, std::ios::end);
        int length = is.tellg();
        char* pszBuf = new char[length+1];
        is.seekg(0, std::ios::beg);
        is.read(pszBuf, length);
        if ( bZip ){
            unsigned int nLen = length * 20;
            if ( nLen < 1024 * 1024 )
                nLen = 1024 * 1024;
            
            char* destBuf = new char[nLen];
            int nRet = BZ2_bzBuffToBuffDecompress(destBuf, &nLen, pszBuf, length, 0, 0);
            if ( nRet == BZ_OK ){
                destBuf[nLen] = 0;
                strText = destBuf;
                delete destBuf;
                bOK = true;
            } else {
                std::cout << "!!!!!!!! BZ2_bzBuffToBuffDecompress " << filename << " error. code = " << nRet << std::endl;
            }
            
        } else {
            pszBuf[length] = 0;
            strText = pszBuf;
            bOK = true;
        }
        
        is.close();

        delete[] pszBuf;
    }

    return bOK;
}


struct OddsTrendParams
{
    int makerid;
    int matchid;
    int groupid;
    std::string strFullFileName;
    
}; // struct OddsTrendParams


class GetOddsTrendHtmlThread : public ucc::JoinableThread
{
public:
    GetOddsTrendHtmlThread(const OddsTrendParams& params){
        m_params = params;
    }

    virtual void run(){
        m_mutex.lock();
        
        std::string url = "http://data.okooo.com/MatchInfo/MatchReportOddsTrend.php";
        int makerid = m_params.makerid;
        int matchid = m_params.matchid;
        int groupid = m_params.groupid;
        std::string strFullFileName = m_params.strFullFileName;

        std::cout << "Enter GetOddsTrendHtmlThread::run() " << " makerid = " << makerid << " matchid = " << matchid << " groupid = " << groupid << std::endl;
        
        
        UrlGet::FormVariables fvs;
        std::stringstream ss;
        ss.str("");
        ss << makerid;
        fvs.push_back(std::make_pair("MakerID", ss.str().c_str()));
        ss.str("");
        ss << matchid;
        fvs.push_back(std::make_pair("MatchID", ss.str().c_str()));
        ss.str("");
        ss << groupid;
        fvs.push_back(std::make_pair("GroupID", ss.str().c_str()));        

        std::cout << "==-== Post URL ==-== " << url << std::endl;
        UrlGet ug;
        if ( !ug.PostUrl(url, fvs) ){
            std::cout << " failed." << std::endl;
        }else{
            std::cout << " OK." << std::endl;
                
            std::string strHtmlGB = ug.GetDoc();
            std::string strHtml = Encoding::GBToUTF8(strHtmlGB);

            SaveFile(strFullFileName, strHtml, true);
            std::cout << strFullFileName.c_str() << std::endl;
        }

        m_mutex.unlock();
        
    }
    
private:
    OddsTrendParams m_params;
    ucc::mutex m_mutex;
    
}; // class GetOddsTrendHtmlThread


// ==================== class Group ====================

class Group
{
public:
    int id;
    std::string name;

    Group(){
        id = -1;
    }
    
    void Clear(){
        id = -1;
        name = "";
        m_terms.clear();
    }
    
    void AddTerm(int termId){m_terms.push_back(termId);};
    size_t GetTermsCount() const{return m_terms.size();};
    int GetTermByIndex(size_t idx) const{return m_terms[idx];};
    bool HasTerm(int termid) const{
        for ( std::vector<int>::const_iterator it = m_terms.begin() ; it != m_terms.end() ; it++ ){
            if ( *it == termid ) return true;
        }
        return false;
    }
    
private:
    std::vector<int> m_terms;
}; // class Group


class Term
{
public:
    int id;
    std::string name;
    int groupid;

    Term(){
        id = -1;
        groupid = -1;
    }
    
    
}; // class Term

    
// ==================== class Match ====================

class Match
{
public:
    int id;
    int groupid;
    int turnid;
    int homeid;
    int awayid;
    int homegoal;
    int awaygoal;
    int matchstate; // 1 : End
    
    std::string matchDate;
    std::string matchTime;

    std::string groupName;
    std::string homeName;
    std::string awayName;
    
    Match(){
        Clear();
    }
    
    void Clear(){
        id = -1;
        groupid = -1;
        turnid = -1;
        homeid = -1;
        awayid = -1;
        homegoal = 0;
        awaygoal = 0;
        matchstate = 0; // End 1, Post -1, Not 0

        matchDate = "";
        matchTime = "";

        groupName = "";
        homeName = "";
        awayName = "";
        
    }

    void GetMatchDate(int& year, int& month, int& day){
        if ( matchDate.length() == 10 ){
            std::string y = matchDate.substr(0, 4);
            std::string m = matchDate.substr(5, 2);
            std::string d = matchDate.substr(8, 2);
            year = atoi(y.c_str());
            month = atoi(m.c_str());
            day = atoi(d.c_str());
        }
    }

    void Print() const{
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
        std::cout << "matchid: " << id << std::endl;
        
        std::cout << groupName << "(" << groupid << ")\t第" << turnid << "轮\t" << matchTime << "\t"
                  << homeName << "(" << homeid << ")\t";
        
        if ( matchstate == 1 ) {        
            std::cout << homegoal << ":" << awaygoal << "\t";

        } else if ( matchstate == -1 ) {
            std::cout << "!!延期!!\t";
        } else {
            std::cout << "!!未赛!!\t";
        }

        std::cout << awayName << "(" << awayid << ") ";

        std::cout << std::endl;

        std::cout << "--------------------------------------------------------------------------------" << std::endl;        
    }
    
}; // class Match

typedef std::map<int, Group> GroupMap;
typedef std::map<int, Term> TermMap;
typedef std::map<int, Match> MatchMap;

GroupMap groups; // 全部联赛
TermMap terms; // 全部球队
MatchMap matches; // 全部比赛详细信息

typedef std::vector<int> Matches; // 比赛集合
typedef std::map<int, Matches> TurnMatches; // 每轮的比赛集合
typedef std::map<int, TurnMatches> GroupMatches; // 每个联赛的比赛集合

GroupMatches groupMatches; // 按联赛分组的比赛集合

typedef std::map<std::string, Matches> DayMatches; // 每天的比赛集合
DayMatches dayMatches; // 按日期分组的比赛集合

const char* concedevalues[] =
{
    "平手", // 0
    "平半", // 1
    "半球", // 2
    "半一", // 3
    "一球", // 4
    "一球球半", // 5
    "球半", // 6
    "球半两球", // 7
    "两球", // 8
    "两球两球半", // 9
    "两球半", // 10
    "两球半三球", // 11
    "三球", // 12
    "三球三球半", // 13
    "三球半", // 14
    "三球半四球", // 15
    "四球", // 16
    "四球四球半", // 17
    "四球半", // 18
    "四球半五球", // 19
    "五球", // 20
    "五球五球半", // 21
    "五球半", // 22
    "五球半六球", // 23
    "六球" // 24
};

#include "ThreadStack.h"
typedef ThreadStack<OddsTrendParams, GetOddsTrendHtmlThread> OddsTrendHtmlThreadStack;

// ==================== class LuckyGoal ====================

class LuckyGoal
{
private:
    //OddsTrendHtmlThreadStack m_oddsTrendHtmlThreadStack;

    std::vector<int> m_hotBankers;

    typedef boost::match_results<std::string::const_iterator> RegexResultsIterator;

    size_t RegexSearch(const std::string strText, const boost::regex& e, RegexResultsIterator& what){
        if ( strText.empty() ) return 0;
        
        std::string::const_iterator start = strText.begin();
        std::string::const_iterator end = strText.end();
        boost::match_flag_type flags = boost::match_default;
      
        if ( boost::regex_search(start, end, what, e, flags) ){
            return 1;
        } else {
            std::cout << "No match. expression = " << e << std::endl;
            return 0;
        }
    }

    typedef std::vector<RegexResultsIterator> RegexResultsIterators;    
    size_t RegexSearch(const std::string strText, const boost::regex& e, RegexResultsIterators& whats){
        if ( strText.empty() ) return 0;
        
        std::string::const_iterator start = strText.begin();
        std::string::const_iterator end = strText.end();
        boost::match_flag_type flags = boost::match_default;

        RegexResultsIterator what;
        size_t cnt = 0;
        while( boost::regex_search(start, end, what, e, flags) ){
            whats.push_back(what);
            cnt++;

            start = what[0].second - 1;
        }

        return cnt;
    }

    
    bool SearchGroupName(const std::string& strText){
        if ( strText.empty() ) return false;
        
        boost::regex e("<td.*>[[:space:]]*(\\S+)第[[:space:]]*<select name='TurnID'");
        RegexResultsIterator what;
    
        if( RegexSearch(strText, e, what) > 0 ){
            std::string groupName = std::string(what[1].first, what[1].second);
          
            std::cout << "Group: [" << groupName << "]" << std::endl;

            return true;
        }

        return false;
    }

    void SearchTerms(const std::string& strText){
        if ( strText.empty() ) return;
        
        // ? 用 . 代替
        boost::regex e("<td><a href=\"/Team/TeamInfoIndex.php.TeamID=(\\d+)\" target=\"_blank\" class=\"BlueWord_TxtL\">(\\S+)</a><br /></td>");

        RegexResultsIterators whats;
        size_t nTerms = RegexSearch(strText, e, whats);
        for ( size_t i = 0 ; i < nTerms ; i++ ){
            RegexResultsIterator what = whats[i];
            std::string termId = std::string(what[1].first, what[1].second);
            std::string termName = std::string(what[2].first, what[2].second);

            std::cout << "Term id: " << termId << " name: " << termName << std::endl;
        }
    }

private:
    Match m_match;
    std::string m_groupName;
    std::string m_matchTime;

    std::string m_termName1;
    int m_termId1;
    int m_groupId1;
    int m_goal1;
    
    std::string m_termName2;
    int m_termId2;
    int m_groupId2;
    int m_goal2;

    void TryAddNewGroup(int id, const std::string& name){
        if ( groups.find(id) != groups.end() ){
            Group group;
            group.id = id;
            group.name = name;
            groups.insert(GroupMap::value_type(id, group));
        }
    }

    void TryAddNewTerm(int id, const std::string& name, int groupid){
        GroupMap::iterator it = groups.find(groupid);
        if ( it != groups.end() ){
            Group& group = (*it).second;
            if ( !group.HasTerm(id) ){
                group.AddTerm(id);
            }
        }

        if ( terms.find(id) != terms.end() ){
            Term term;
            term.id = id;
            term.name = name;
            
            terms.insert(TermMap::value_type(id, term));
        }
    }

    std::string GetDayMatchesDir(int year, int month) const{
        char buf[11];
        sprintf(buf, "%04d/%02d", year, month);
        buf[10] = 0;
        return std::string(buf);
    }
    
    std::string GetDayMatchesHtmlFileName(int year, int month, int day) const{
        char buf[11];
        sprintf(buf, "%04d-%02d-%02d", year, month, day);
        buf[10] = 0;
        std::string matchdate(buf);
        
        std::stringstream ss;
        ss << "match-" << matchdate << ".html";
        return ss.str();
    }
    
    void SaveDayMatchesHtml(int year, int month, int day, const std::string& strHtml) const{
        std::string filename = GetDayMatchesHtmlFileName(year, month, day);
        std::fstream of;
        of.open(filename.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
        if ( of.is_open() ){
            of.write( strHtml.c_str(), strHtml.length());
            of.close();
        }
    }

    std::string GetHtmlFromCacheOrUrl(const std::string& strFullFileName, const std::string& url ) const{
        std::string strHtml;
        if ( !LoadFile(strFullFileName, strHtml, true) ){
            strHtml = GetHtmlByUrl(url);
            SaveFile(strFullFileName, strHtml, true);
        }
        return strHtml;
    }

    std::string GetDayMatchHtmlFromCacheOrUrl(int year, int month, int day) const{
        std::string strFullFileName = GetDayMatchFullFileName(year, month, day);
        std::string url = GetDayMatchUrl(year, month, day);

        return GetHtmlFromCacheOrUrl(strFullFileName, url);
    }

    std::string GetHandicapHtmlFromCacheOrUrl(int matchid, int year, int month, int day) const {
        std::string filename = GetHandicapFullFileName(matchid, year, month, day);
        std::string url = GetHandicapUrl(matchid);

        return GetHtmlFromCacheOrUrl(filename, url);
    }

    std::string GetHandicapTrendHtmlFromCacheOrUrl(int matchid, int makerid, int year, int month, int day) const {
        std::string filename = GetHandicapTrendFullFileName(matchid, makerid, year, month, day);
        std::string url = GetHandicapTrendUrl(matchid, makerid);

        return GetHtmlFromCacheOrUrl(filename, url);
    }
    
    std::string GetLastestOddsHtmlFromCacheOrUrl(int matchid, int year, int month, int day) const {
        std::string filename = GetLastestOddsFullFileName(matchid, year, month, day);
        std::string url = GetLastestOddsUrl(matchid);

        return GetHtmlFromCacheOrUrl(filename, url);
    }

    std::string GetOddsTrendHtmlFromCacheOrUrl(int matchid, int makerid, int groupid, int year, int month, int day){
        std::string strFullFileName = GetOddsTrendFullFileName(matchid, makerid, year, month, day);
        std::string strHtml;
        std::cout << "Try to find " << strFullFileName << std::endl;
        
        if ( !LoadFile(strFullFileName, strHtml, true) ){
            // OddsTrendParams params;
            // params.makerid = makerid;
            // params.matchid = matchid;
            // params.groupid = groupid;
            // params.strFullFileName = strFullFileName;
            
            // m_oddsTrendHtmlThreadStack.Push(params);
            
            std::string url = "http://data.okooo.com/MatchInfo/MatchReportOddsTrend.php";

            std::cout << "Not found! Get html by url. " << std::endl;
            
            UrlGet::FormVariables fvs;
            std::stringstream ss;
            ss.str("");
            ss << makerid;
            fvs.push_back(std::make_pair("MakerID", ss.str().c_str()));
            ss.str("");
            ss << matchid;
            fvs.push_back(std::make_pair("MatchID", ss.str().c_str()));
            ss.str("");
            ss << groupid;
            fvs.push_back(std::make_pair("GroupID", ss.str().c_str()));        

        
            std::cout << "==-== Post URL ==-== " << url << std::endl;
            UrlGet ug;
            if ( !ug.PostUrl(url, fvs) ){
                std::cout << " failed." << std::endl;
                return "";
            }else{
                std::cout << " OK." << std::endl;
                
                std::string strHtmlGB = ug.GetDoc();
                std::string strHtml = Encoding::GBToUTF8(strHtmlGB);

                SaveFile(strFullFileName, strHtml, true);
                std::cout << strFullFileName.c_str() << std::endl;
            }
        }
        
        return strHtml;
    }
    
    // --------------------------
    // 获得每月比赛本地网页文件的全路径
    // --------------------------
    std::string GetMonthMatchesFullDir(int year, int month) const {
        std::string strDayMatchesDir = GetDayMatchesDir(year, month);
        std::stringstream ss0;
        ss0 << std::string("data/pages/matches/") << strDayMatchesDir;
        std::string strMatchesDir = ss0.str();

        return strMatchesDir;
    }
    
    // --------------------------------------
    // 获得指定日期的所有比赛列表的本地网页文件全路径名
    // --------------------------------------
    std::string GetDayMatchFullFileName(int year, int month, int day) const {
        std::string strMonthMatchesDir = GetMonthMatchesFullDir(year, month);
        std::string filename = GetDayMatchesHtmlFileName(year, month, day);

        std::string strFullFileName = strMonthMatchesDir + "/" + filename;
        return strFullFileName;
    }

    // -----------------------------
    // 获得指定日期的所有比赛列表的网页地址
    // -----------------------------
    std::string GetDayMatchUrl(int year, int month, int day) const {
        std::stringstream ss;
        ss << std::string("http://data.okooo.com/MatchInfo/MatchesRecentDays.php?year=")
           << year << "&month=" << month << "&day=" << day;
        std::string url = ss.str();

        return url;
    }

    // ----------------------------------------
    // 获得指定比赛的亚洲盘口列表的本地网页文件的全路径名
    // ----------------------------------------
    std::string GetHandicapFullFileName(int matchid, int year, int month, int day) const {
        std::string strMonthMatchesDir = GetMonthMatchesFullDir(year, month);

        std::stringstream ss;
        ss << strMonthMatchesDir << "/Handicap/"
           << std::setfill('0') << std::setw(2) << day
           << "/handicap_" << matchid << ".html";
        std::string filename = ss.str();

        return filename;
    }
    
    // -----------------------------
    // 获得指定比赛的亚洲盘口列表的网页地址
    // -----------------------------
    std::string GetHandicapUrl(int matchid) const {
        std::stringstream ss;
        ss << std::string("http://data.okooo.com/MatchInfo/MatchReportHandicap.php?MatchID=") << matchid;
        std::string url = ss.str();

        return url;
    }

    // -----------------------------
    // 获得指定比赛、庄家的亚洲盘口历史列表的本地网页文件的全路径名
    // -----------------------------
    std::string GetHandicapTrendFullFileName(int matchid, int makerid, int year, int month, int day) const {
        std::string strMonthMatchesDir = GetMonthMatchesFullDir(year, month);

        std::stringstream ss;
        ss << strMonthMatchesDir << "/Handicap/"
           << std::setfill('0') << std::setw(2) << day
           << "/handicap_" << matchid << "_" << makerid << ".html";
        std::string filename = ss.str();

        return filename;
    }

    // -----------------------------
    // 获得指定比赛、庄家的亚洲盘口历史列表的网页地址
    // -----------------------------
    std::string GetHandicapTrendUrl(int matchid, int makerid) const {
        std::stringstream ss;
        ss << std::string("http://data.okooo.com/MatchInfo/MatchHandicapTrend.php?MatchID=") << matchid
           << std::string("&MakerID=") << makerid;
        std::string url = ss.str();

        return url;
    }

    // ----------------------------------------
    // 获得指定比赛的亚洲盘口列表的本地网页文件的全路径名
    // ----------------------------------------
    std::string GetLastestOddsFullFileName(int matchid, int year, int month, int day) const {
        std::string strMonthMatchesDir = GetMonthMatchesFullDir(year, month);

        std::stringstream ss;
        ss << strMonthMatchesDir << "/LastestOdds/"
           << std::setfill('0') << std::setw(2) << day
           << "/LastestOdds_" << matchid << ".html";
        std::string filename = ss.str();

        return filename;
    }

    // -----------------------------
    // 获得指定比赛的欧洲盘口列表的网页地址
    // -----------------------------
    std::string GetLastestOddsUrl(int matchid) const {
        std::stringstream ss;
        ss << std::string("http://data.okooo.com/MatchInfo/MatchReportLatestOdds.php?MatchID=") << matchid;
        std::string url = ss.str();

        return url;
    }

    // -----------------------------
    // 获得指定比赛、庄家的欧洲盘口历史列表的本地网页文件的全路径名
    // -----------------------------
    std::string GetOddsTrendFullFileName(int matchid, int makerid, int year, int month, int day) const {
        std::string strMonthMatchesDir = GetMonthMatchesFullDir(year, month);

        std::stringstream ss;
        ss << strMonthMatchesDir << "/LastestOdds/"
           << std::setfill('0') << std::setw(2) << day
           << "/lastestodds_" << matchid << "_" << makerid << ".html";
        std::string filename = ss.str();

        return filename;
    }

    // -----------------------------
    // 获得指定比赛、庄家的欧洲盘口历史列表的网页地址
    // -----------------------------
    std::string GetOddsTrendUrl(int matchid, int makerid) const {
        std::stringstream ss;
        ss << std::string("http://data.okooo.com/MatchInfo/MatchHandicapTrend.php?MatchID=") << matchid
           << std::string("&MakerID=") << makerid;
        std::string url = ss.str();

        return url;
    }
    
public:
    LuckyGoal(){
        m_hotBankers.push_back(84);   // 澳门彩票
        m_hotBankers.push_back(35);   // 易胜博
        m_hotBankers.push_back(369);  // YSB 88 (Handicap)
        m_hotBankers.push_back(322);  // 金宝博
        m_hotBankers.push_back(65);   // 伟德国际
        m_hotBankers.push_back(197);  // 互博(Hooball)
        m_hotBankers.push_back(82);   // 立博
        m_hotBankers.push_back(19);   // 必发
        m_hotBankers.push_back(250);  // 皇冠(Singbet)
        m_hotBankers.push_back(27);   // Bet365
        m_hotBankers.push_back(258);  // 波音(cp080)

        std::sort(m_hotBankers.begin(), m_hotBankers.end());

        //m_oddsTrendHtmlThreadStack.start();
    }
    
    // 从"赛事日历"中获取每天的比赛信息
    void SearchMatches(int year, int month, int day){
        
        std::string strHtml = GetDayMatchHtmlFromCacheOrUrl(year, month, day);
        if ( strHtml.empty() ) return;

        RegexResultsIterators whats;
        size_t nTRs =
            RegexSearch(strHtml,
//                        boost::regex("<tr height=\"\\d+\" align=\"center\" class=\"BlackWords WhiteBg\">(.*?)</tr>"),
                        boost::regex("<tr height=\"\\d+\" align=\"center\" [^>]+>(.*?)</tr>"),
                        whats);
        std::cout << "nTRs = " << nTRs << std::endl;
        
        for ( size_t i = 0 ; i < nTRs ; i++ ){
            RegexResultsIterator what = whats[i];            
            std::string strTR = std::string(what[1].first, what[1].second);
            if ( strTR.empty() ) continue;
            
            SearchMatchTitle(strTR);
            SearchMatch(strTR, year, month, day);
        }
    }

    bool SearchMatchTitle(const std::string& strText){
        if ( strText.empty() ) return false;
        
        boost::regex e("<td width=\"20%\" class=\"WhiteText\" bgcolor=\"#\\S+\">\\s*(\\S+)\\s*<br />\\s*([\\S-]+ [\\S:]+)</td>");
        RegexResultsIterator what;
        if ( RegexSearch(strText, e, what) > 0 ){
            std::string groupName = std::string(what[1].first, what[1].second);
            std::string matchTime = std::string(what[2].first, what[2].second);
            //std::cout << groupName << "\t(" << matchTime << ") " ;//<< std::endl;

            m_groupName = groupName;
            m_matchTime = matchTime;

            return true;
        } else
            return false;
    }
    
private:
    void SearchMatch(const std::string& strText, int year, int month, int day) {
        if ( strText.empty() ) return;
        
        boost::regex e("TeamName=(\\S+)&TeamID=(\\d+)&GroupID=(\\d+)(.*?)TeamName=(\\S+)&TeamID=(\\d+)&GroupID=(\\d+)");
        
        RegexResultsIterators whats;
        size_t nMatches = RegexSearch(strText, e, whats);
        for ( size_t i = 0 ; i < nMatches ; i++ ){
            RegexResultsIterator what = whats[i];

            std::string termName = std::string(what[1].first, what[1].second);
            std::string termId = std::string(what[2].first, what[2].second);
            std::string groupId = std::string(what[3].first, what[3].second);

            std::string matchInfo = std::string(what[4].first, what[4].second);
            
            std::string termName2 = std::string(what[5].first, what[5].second);
            std::string termId2 = std::string(what[6].first, what[6].second);
            std::string groupId2 = std::string(what[7].first, what[7].second);

            m_termName1 = termName;
            m_termId1 = atoi(termId.c_str());
            m_groupId1 = atoi(groupId.c_str());
            m_termName2 = termName2;
            m_termId2 = atoi(termId2.c_str());
            m_groupId2 = atoi(groupId2.c_str());
            
            //std::cout << termName << "(" << termId << ":" << groupId << ") ";


            
            //std::cout << termName2 << "(" << termId2 << ":" << groupId2 << ") ";
            
            bool bOK = SearchMatchInfo(matchInfo);

            std::cout << std::endl;

            if ( bOK ){
                // ---------------------
                // 获得比赛的亚洲盘口列表网页
                // ---------------------
                std::string strHandicapHtml = GetHandicapHtmlFromCacheOrUrl(m_match.id, year, month, day);
                //std::cout << strHandicapHtml << std::endl;
                
                SearchHandicap(strHandicapHtml, year, month, day);

                // ---------------------
                // 获得比赛的欧洲盘口列表网页
                // ---------------------
                std::string strLastestOddsHtml = GetLastestOddsHtmlFromCacheOrUrl(m_match.id, year, month, day);
                //SearchLastestOdds(strLastestOddsHtml, year, month, day);
                
            }
        }
        
    }

    void SearchHandicap(const std::string& strHandicapHtml, int year, int month, int day){
        if ( strHandicapHtml.empty() ) return;
        
        // <tr id="MakerID_84" class="WhiteBg" align="center">
        // 						<td align="left">MakerName</td>
        // 						<td> 0.72 </td>
        // 						<td class="BgGrey">HandicapName</td>
        // 						<td> 1.04 </td>
        // 						<a href="/MatchInfo/MatchReportHandicpTrend.php?MakerID=84&HomeID=1224&AwayID=675&MatchID=116892&GroupID=1208" target="_blank">
        // 						<td style="cursor:pointer;">0.72</td>
        // 						<td style="cursor:pointer;" class="BgGrey"><span class="BlackText" >HandicapName</span></td>
        // 						<td style="cursor:pointer;">1.04</td>
        // 						</a>
        // 						<td><a href="/MatchInfo/MatchHandicapTrendCreatePic.php?MatchID=116892&MakerID=84" target="_blank" class="RedUp">走势图</a> <a href="/MatchInfo/MatchHandicapTrend.php?MatchID=116892&MakerID=84" target="_blank" class="RedUp">变化表</a></td>
        // 					</tr>

        boost::regex e("<tr[^>]+>\\s*<td[^>]*>([^<]+)</td>\\s*<td>\\s*(\\S+)\\s*</td>\\s*<td[^>]*>(\\S+)</td>\\s*<td>\\s*(\\S+)\\s*</td>\\s*<a href=\"/MatchInfo/MatchReportHandicpTrend.php.MakerID=(\\d+)[^>]+>\\s*<td[^>]+>(<span[^>]+>)?(\\d.\\d\\d)\\S*(</span>)?</td>\\s*<td[^>]+>(<span[^>]+>)?([^<]+)(</span>)?</td>\\s*<td[^>]+>(<span[^>]+>)?(\\d.\\d\\d)\\S*(</span>)?</td>(.*?)</tr>");
        RegexResultsIterators whats;
        size_t nMakers = RegexSearch(strHandicapHtml, e, whats);
        for ( size_t i = 0 ; i < nMakers ; i++ ){
            RegexResultsIterator what = whats[i];
            std::string strMakerName = std::string(what[1].first, what[1].second);
            std::string strHomeWater0 = std::string(what[2].first, what[2].second);
            std::string strConcede0 = std::string(what[3].first, what[3].second);            
            std::string strAwayWater0 = std::string(what[4].first, what[4].second);            
            std::string strMakerId = std::string(what[5].first, what[5].second);
            
            std::string strHomeWater1 = std::string(what[7].first, what[7].second);            

            std::string strConcede1 = std::string(what[10].first, what[10].second);            

            std::string strAwayWater1 = std::string(what[13].first, what[13].second);            

            int makerid = atoi(strMakerId.c_str());

            std::cout << strMakerName << "("  << strMakerId << ")"
                      << " \t" << strHomeWater0
                      << " \t" << strConcede0
                      << " \t" << strAwayWater0
                      << " -> " << strHomeWater1
                      << " \t" << strConcede1
                      << " \t" << strAwayWater1
                      << std::endl
                      << std::endl;
            
            if ( std::binary_search(m_hotBankers.begin(), m_hotBankers.end(), makerid) ){
                // ---------------------
                // 获得指定比赛、庄家的亚洲盘口历史列表网页
                // ---------------------
                std::string strHandicapTrendHtml = GetHandicapTrendHtmlFromCacheOrUrl(m_match.id, makerid, year, month, day);
                
            }
        }
    }

    // --------------
    // 检索欧赔重要数据
    // --------------
    void SearchLastestOdds(const std::string& strLastestOddsHtml, int year, int month, int day){
        std::cout << "Enter SearchLastestOdds()" << std::endl;
        if ( strLastestOddsHtml.empty() ) return;
        
        boost::regex e("\\[([[:digit:]]+)\\] = '.*?';");
        
        RegexResultsIterators whats;
        size_t nMakers = RegexSearch(strLastestOddsHtml, e, whats);
        std::cout << "Odds makers count = " << nMakers << std::endl;
        
        for ( size_t i = 0 ; i < nMakers ; i++ ){
            RegexResultsIterator what = whats[i];
            std::string strMakerId = std::string(what[1].first, what[1].second);

            int makerid = atoi(strMakerId.c_str());
            if ( std::binary_search(m_hotBankers.begin(), m_hotBankers.end(), makerid) ){
                std::cout << "Maker: " << makerid << std::endl;
                GetOddsTrendHtmlFromCacheOrUrl(m_match.id, makerid, m_match.groupid, year, month, day);
            }
        }
    }

    bool SearchScore(const std::string& matchInfo){
        boost::regex e("<a href=\"/MatchInfo/MatchGoalsShow.php.MatchID=\\d+\" target=\"_blank\"><span class=\"BlackStrong\">(\\d+)\\D+(\\d+)</span>");
        RegexResultsIterator what;
        if ( RegexSearch(matchInfo, e, what) > 0 ){
            std::string goal1(what[1].first, what[1].second);
            std::string goal2(what[2].first, what[2].second);

            m_goal1 = atoi(goal1.c_str());
            m_goal2 = atoi(goal2.c_str());
            
            //std::cout << " " << goal1 << ":" << goal2 << " ";

            return true;
        } else
            return false;
    }
    

    bool SearchMatchInfo(const std::string& matchInfo){
        if ( matchInfo.empty() ) return false;
        
        boost::regex e("/MatchInfo/MatchReportHandicap.php.MatchID=(\\d+)&GroupID=(\\d+)&TurnID=(\\d+)&HomeID=(\\d+)&AwayID=(\\d+)&MatchDate=(\\d\\d\\d\\d-\\d\\d-\\d\\d)&MatchState=(\\w*)\"");

        RegexResultsIterator what;
        if ( RegexSearch(matchInfo, e, what) > 0 ){
            std::string matchId = std::string(what[1].first, what[1].second);
            std::string groupId = std::string(what[2].first, what[2].second);
            std::string turnId = std::string(what[3].first, what[3].second);
            std::string homeId = std::string(what[4].first, what[4].second);
            std::string awayId = std::string(what[5].first, what[5].second);
            std::string matchDate = std::string(what[6].first, what[6].second);
            std::string matchState = std::string(what[7].first, what[7].second);

            int gId = atoi(groupId.c_str());
            TryAddNewGroup(gId, m_groupName);

            TryAddNewTerm(m_termId1, m_termName1, gId);
            TryAddNewTerm(m_termId2, m_termName2, gId);
            
            int homeid = atoi(homeId.c_str());
            int awayid = atoi(awayId.c_str());
            int turnid = atoi(turnId.c_str());

            m_match.Clear();
            m_match.id = atoi(matchId.c_str());
            m_match.groupid = gId;
            m_match.turnid = turnid;
            m_match.homeid = homeid;
            m_match.awayid = awayid;

            m_goal1 = 0;
            m_goal2 = 0;
            if ( matchState == "End" ){
                m_match.matchstate = 1;
                SearchScore(matchInfo);                
            } else if ( matchState == "Post" ) {
                m_match.matchstate = -1;
            } //else if ( matchState == "Not" ){
            //    m_match.matchstate = 0;
            //}
            

            if ( homeid == m_termId1 ){
                m_match.homegoal = m_goal1;
                m_match.awaygoal = m_goal2;
                m_match.homeName = m_termName1;
                m_match.awayName = m_termName2;
            } else {
                m_match.homegoal = m_goal2;
                m_match.awaygoal = m_goal1;
                m_match.homeName = m_termName2;
                m_match.awayName = m_termName1;
            }
            
            m_match.matchTime = m_matchTime;
            m_match.matchDate = matchDate;

            m_match.groupName = m_groupName;

            m_match.Print();
            
            // std::cout << " m: " << matchId
            //           << " g: " << groupId
            //           << " t: " << turnId
            //           << " h: " << homeId
            //           << " a: " << awayId
            //           << " d: " << matchDate
            //           << " s: " << matchState
            //     ;

            return true;
        } else
            return false;
    }
    
public:

    void DoSearch(const std::string& url){
        std::string strHtml = GetHtmlByUrl(url);

        if ( !strHtml.empty() ){
            SearchGroupName(strHtml);
            SearchTerms(strHtml);
        }
    }
    
    
}; // class LuckyGoal


void Usage()
{
    std::cerr << "==--*--== LuckyGoal ==--*--==" << std::endl
              << "Usage: luckygoal g <groupid>" << std::endl
              << "       luckygoal m <year> <month> [day]" << std::endl
              << "       luckygoal s <url> <filename>" << std::endl
              << std::endl;
}

int main(int argc, char *argv[])
{
    if ( argc < 2 ){
        Usage();
        return EXIT_FAILURE;
    }

    std::string cmd = argv[1];

    if ( cmd == "s" ){
        if ( argc >= 4 ){
            std::string url = argv[2];
            std::string filename = argv[3];
            
            std::string strHtml = GetGBHtmlByUrl(url);
            if ( strHtml.length() > 0 ){
                std::fstream of;
                of.open(filename.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
                if ( of.is_open() ){
                    of.write( strHtml.c_str(), strHtml.length());
                    of.close();
                }
            }
            return 0;
        }
    } else if ( cmd == "g" ) {
        std::string groupid = "1218"; // 德甲2008-2009赛季
        if ( argc >= 3 ){
            groupid = argv[2];
        }
    
        std::stringstream ss;
        ss << std::string("http://data.okooo.com/League/LeagueInfoIndex.php?GroupID=") << groupid;
        std::string url = ss.str();

        LuckyGoal lg;
        lg.DoSearch(url);

        return 0;
    } else if ( cmd == "o" ){
        UrlGet::FormVariables fvs;
        fvs.push_back(std::make_pair("MakerID", "19"));
        fvs.push_back(std::make_pair("MatchID", "130567"));
        fvs.push_back(std::make_pair("GroupID", "1450"));        
        std::string url = "http://data.okooo.com/MatchInfo/MatchReportOddsTrend.php";
        
        UrlGet ug;
        if ( !ug.PostUrl(url, fvs) ){
            std::cout << "==!==Post URL: " << std::string(url) << "failed. ==!==" << std::endl;
        }else{
            std::string strHtmlGB = ug.GetDoc();
            std::string strHtml = Encoding::GBToUTF8(strHtmlGB);

            std::cout << strHtml << std::endl;
            std::cout << "==-== Get URL OK. ==-==" << url << std::endl;
        }

        return 0;
    } else if ( cmd == "m" ){
        if ( argc >= 3 ){
            int year = atoi(argv[2]);            
            if ( argc >= 4 ){
                int month = atoi(argv[3]);

                if ( argc >= 5 ){

                    // one day data.
                    
                    int day = atoi(argv[4]);

                    LuckyGoal lg;
                    lg.SearchMatches(year, month, day);
                } else {

                    // one month data
                    
                    int days = Date::GetMonthDays(year, month);
                    for ( int day = 1 ; day <= days ; day++ ){
                        LuckyGoal lg;
                        lg.SearchMatches(year, month, day);
                    }
                }
            } else {

                // one year data
                
                for ( int month = 1 ; month <= 12 ; month++ ){
                    int days = Date::GetMonthDays(year, month);
                    for ( int day = 1 ; day <= days ; day++ ){                    
                        LuckyGoal lg;
                        lg.SearchMatches(year, month, day);
                    }
                }
            }
            return 0;
        }
        
    }

    Usage();
    
    return 0;
  
}
