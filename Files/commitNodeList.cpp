#include <iostream>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <ctime>
#include <vector>

using namespace std;

// ================  Global Functions  ==================

//to generate random commit ids upon each commit:
string gen_random(const int len)
{
    //to provide a random seed to rand() fn such that it generates a random set of values each len times;
    srand((unsigned)time(NULL) * getpid());
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i){
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

//get the current time in the format yyyy/mm/dd/hh/mm, year is given from 1900 so 1900 is added, month is given zero-based so 1 is added:
string get_time()
{
    time_t t = std::time(0); // get time now
    tm *now = std::localtime(&t);
    string dateTime = to_string(now->tm_year + 1900) + "/" +
                        to_string(now->tm_mon + 1) + "/" +
                        to_string(now->tm_mday) + " " +
                        to_string(now->tm_hour) + ":" +
                        to_string(now->tm_min) + "\n";

    return dateTime;
}

// ======================commitNode class===================================>

class commitNode{
private:
    //properties associated with a commit:
    string commitID;
    string commitMsg;
    string nextCommitID;
    commitNode *next;

    void createCommitNode()
    {
        // create commit dir, create commitInfo.txt, copy files
        filesystem::create_directory(filesystem::current_path() / ".git" / "commits" / commitID);

        auto path = filesystem::current_path() / ".git" / "commits" / commitID / "commitInfo.txt";

        //writing information of the commit into the txt file:
        ofstream write(path.string());
        write << "1." + commitID + "\n" +
                    "2." + commitMsg + "\n" +
                    "3." + get_time() + "\n";

        //specifying directory path for the staging area:
        auto STAGING_AREA_PATH = filesystem::path(filesystem::current_path() / ".git" / "staging_area");

        //setting copy options for the directory:
        const auto copyOptions = filesystem::copy_options::update_existing | filesystem::copy_options::recursive;
        filesystem::copy(STAGING_AREA_PATH, filesystem::current_path() / ".git" / "commits" / commitID / "Data", copyOptions);
    }

public:
    //Constructors------------------------------------------------->
    commitNode(){
        this->next = NULL;
    }
    commitNode(string commitID, string commitMsg)
    {
        this->commitID = commitID;
        this->commitMsg = commitMsg;
        this->next = NULL;
        createCommitNode();
    }
    commitNode(string commitId)
    {
        // msg in not populated.
        checkNextCommitId();
        this->commitID = commitId;
        next = NULL;
    }

    //Class methods (functions)------------------------------------>

    //to check for the next commit id, if exists:
    string checkNextCommitId()
    {
        filesystem::path tempPath(filesystem::current_path() / ".git" / "commits" / getCommitID() / "nextCommitInfo.txt");
        bool exists = filesystem::exists(tempPath);
        if (exists){
            string info;
            ifstream file(tempPath);
            while (getline(file, info))
            {
                //first and only line contains the nextCommitId:
                this->nextCommitID = info;
                break;
            }
            file.close();
            return nextCommitID;
        }
        return "";
    }

    //to checkout (revert to an older version of a file):
    void revertCommitNode(string fromHash)
    {
        filesystem::create_directories(filesystem::current_path() / ".git" / "commits" / getCommitID() / "Data");
        auto nextCommitPath = filesystem::current_path() / ".git" / "commits" / getCommitID() / "commitInfo.txt";
        auto copyFrom = filesystem::current_path() / ".git" / "commits" / fromHash / "Data";

        //writing current details into the next commit, in case user wants to go back:
        ofstream write(nextCommitPath.string());
        write << "1." + commitID + "\n" +
                    "2." + commitMsg + "\n" +
                    "3." + get_time() + "\n";
        const auto copyOptions = filesystem::copy_options::recursive;
        filesystem::copy(copyFrom, filesystem::current_path() / ".git" / "commits" / getCommitID() / "Data", copyOptions);
    }

    //to retrieve commitid and commitmsg for a node:
    void readNodeInfo()
    {
        string info;
        auto path = filesystem::current_path() / ".git" / "commits" / getCommitID() / "commitInfo.txt";
        ifstream file(path.string());
        while (getline(file, info))
        {
            if (info[0] == '1')
            {
                this->setCommitID(info.substr(2));
            }
            if (info[0] == '2')
            {
                this->setCommitMsg(info.substr(2));
            }
        }
    }

    //Getter and Setter functions to access private properties outside the class:
    string getCommitMsg()
    {
        return this->commitMsg;
    }
    void setCommitMsg(string commitMsg)
    {
        this->commitMsg = commitMsg;
    }

    void setCommitID(string commitID)
    {
        this->commitID = commitID;
    }
    string getCommitID()
    {
        return this->commitID;
    }

    void setNext(commitNode *node)
    {
        this->next = node;
    }
    commitNode *getNext()
    {
        return next;
    }

    void setNextCommitID(string nextCommitId)
    {
        this->nextCommitID = nextCommitId;
    }
    string getNextCommitId()
    {
        return this->nextCommitID;
    }
    void writeNextCommitID(string nextCommitID)
    {
        setNextCommitID(nextCommitID);
        auto path = filesystem::current_path() / ".git" / "commits" / getCommitID() / "nextCommitInfo.txt";
        ofstream write(path.string());
        //first and only line contains the nextCommitId:
        write << nextCommitID;
    }
};

// ======================commitNodeList class===================================>
class commitNodeList
{
private:
    //head and tail for easy traversals, insertions and deletions:
    commitNode *HEAD;
    commitNode *TAIL;

    // check if HEAD commit exists
    bool checkHead()
    {
        //"0x1111" is chosen as the first(head) commit id:
        auto tempDir = filesystem::current_path() / ".git" / "commits" / "0x1111";
        return filesystem::exists(tempDir);
    }

public:
    //constructor----------------------------------:
    commitNodeList()
    {
        setHead(NULL);
        setTail(NULL);
        if (checkHead())
        {
            setHead(new commitNode("0x1111"));
        }
    }

    //getter and setter fns----------------------:
    commitNode *getHead()
    {
        return this->HEAD;
    }
    void setHead(commitNode *HEAD)
    {
        this->HEAD = HEAD;
    }

    commitNode *getTail()
    {
        return this->TAIL;
    }
    void setTail(commitNode *TAIL)
    {
        this->TAIL = TAIL;
    }

    //methods (functions) on the list------------------------------------:

    //add the commit (file version) to the end of the list:
    void addOnTail(string msg)
    {
        //if first commit, goes onto the head:
        if (!checkHead())
        {
            commitNode *newNode = new commitNode("0x1111", msg);
            setHead(newNode);
        }
        else
        {
            string commitID = gen_random(8);
            commitNode *currNode = getHead();
            while (currNode != NULL)
            {
                string nextCommitId = currNode->checkNextCommitId();

                if (nextCommitId.length() < 8)
                {
                    commitNode *newNode = new commitNode(commitID, msg);
                    currNode->writeNextCommitID(commitID);
                    currNode = NULL;
                }
                else
                {
                    commitNode *newNode = new commitNode();
                    newNode->setCommitID(nextCommitId);
                    currNode = newNode;
                }
            }
        }
    }

    // to checkout:
    void revertCommit(string commitHash)
    {
        commitNode *commitNodeToRevert;
        if (!checkHead()){
            cout << "Nothing to Revert to " << endl;
        }
        else{
            bool error = true;
            string commitID = gen_random(8);
            commitNode *currNode = getHead();
            while (currNode != NULL)
            {
                string nextCommitId = currNode->checkNextCommitId();
                if (commitHash == currNode->getCommitID())
                {
                    commitNodeToRevert = currNode;
                    error = false;
                }

                if (nextCommitId.length() < 8)
                {
                    if (!error)
                    {
                        commitNodeToRevert->readNodeInfo();
                        commitNode *newNode = new commitNode();
                        newNode->setCommitID(commitID);
                        newNode->setCommitMsg(commitNodeToRevert->getCommitMsg());
                        newNode->revertCommitNode(commitNodeToRevert->getCommitID());

                        currNode->writeNextCommitID(commitID);
                    }
                    currNode = NULL;
                }
                else
                {
                    commitNode *newNode = new commitNode();
                    newNode->setCommitID(nextCommitId);
                    currNode = newNode;
                }
            }

            if (error == true)
            {
                cout << "does not match any Commit Hash" << endl;
            }
        }
    }

    // to print all commits till now.. (git log analogous):
    void printCommitList()
    {
        commitNode *currNode = getHead();
        while (currNode != NULL)
        {
            string nextCommitId = currNode->checkNextCommitId();
            filesystem::path commitPath(filesystem::current_path() / ".git" / "commits" / currNode->getCommitID() / "commitInfo.txt");
            string info;
            ifstream file(commitPath.string());
            while (getline(file, info))
            {
                if (info[0] == '1')
                {
                    cout << "Commit ID:    " << info.substr(2) << endl;
                }
                if (info[0] == '2')
                {
                    cout << "Commit Msg:   " << info.substr(2) << endl;
                }
                if (info[0] == '3')
                {
                    cout << "Date & Time:  " << info.substr(2) << endl;
                }
            }
            file.close();
            cout << "============================\n\n";

            if (nextCommitId.length() < 8)
            {
                currNode = NULL;
            }
            else
            {
                commitNode *newNode = new commitNode();
                newNode->setCommitID(nextCommitId);
                currNode = newNode;
            }
        }
    }
};