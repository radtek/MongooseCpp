#include <sstream>
#include <algorithm>
#include "TestTypeController.h"
#include "WebServer.h"
#include "HttpClient.h"
#include "HttpHelper.h"

#include "TypeController.h"

using namespace std;


Book::Book(int _ref, const string& _title, const string& _author, int _stock) : Ref(_ref), Title(_title), Author(_author), Stock(_stock)
{
}

JsonController::JsonController()
{
    Book book1(123112, "Les Fleurs du mal", "Charles Baudelaire", 75);
    Book book2(123342, "Les mis�rables", "Victor Hugo", 68);
    Book book3(123654, "L'�tranger", "Albert Camus", 75);
    Book book4(123759, "Les Liaisons dangereuses", "Choderlos de Laclos", 75);
    Book book5(123987, "Le Petit Prince", "Antoine de Saint-Exup�ry", 75);

    m_Datas.push_back(book1);
    m_Datas.push_back(book2);
    m_Datas.push_back(book3);
    m_Datas.push_back(book4);
    m_Datas.push_back(book5);
}

JsonController::~JsonController()
{
}

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"                 //This is virtual functions
#endif

bool JsonController::GetList(MongooseCpp::Request& request, MongooseCpp::Response& response)
{
    vector<Book>::const_iterator it;
    ostringstream oss;


    oss << "{ Books : [" << endl;
    for(it=m_Datas.begin(); it!=m_Datas.end(); ++it)
    {
        if(it!=m_Datas.begin()) oss << "," << endl;
        oss << "{" << endl;
        oss << "Title:\"" << it->Title << "\"," << endl;
        oss << "Author:\"" << it->Author << "\"," << endl;
        oss << "}";
    }
    oss << "]}" << endl;

    response.SetContent(oss.str());
    return true;
}

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

bool JsonController::Get(const unsigned int& Id, Book& book, MongooseCpp::Response& response)
{
    if(Id==999)
    {
        response.SetStatut(403);
        response.SetContent("Id 999 unauthorized");
        return false;
    }
    if(Id>=m_Datas.size())
    {
        response.SetStatut(404);
        response.SetContent("Book not found");
        return false;
    }
    book = m_Datas[Id];
    return true;
}

bool JsonController::Post(const Book& book, MongooseCpp::Response& response)
{
    ostringstream oss;
    vector<Book>::iterator it = find_if(m_Datas.begin(), m_Datas.end(), [&book] (const Book& bookData) { return bookData.Ref == book.Ref; } );

    if(it!=m_Datas.end())
    {
        oss << "Book ref " << book.Ref << " already exist";
        response.SetContent(oss.str());
        return true;
    }

    m_Datas.push_back(book);
    oss << "New Id:" << m_Datas.size()-1;
    response.SetContent(oss.str());
    return true;
}

bool JsonController::Put(const unsigned int& Id, const Book& book, MongooseCpp::Response& response)
{
    if(Id>=m_Datas.size())
    {
        response.SetStatut(404);
        response.SetContent("Book not found");
        return false;
    }

    m_Datas[Id] = book;

    return true;
}

bool JsonController::Delete(const unsigned int& Id, MongooseCpp::Response& response)
{
    if(Id>=m_Datas.size())
    {
        response.SetStatut(404);
        response.SetContent("Book not found");
        return false;
    }

    m_Datas.erase(m_Datas.begin()+Id);

    return true;
}

string JsonController::ToString(const Book& book)
{
    ostringstream oss;
    oss << "Book {" << endl;
    oss << "Ref:" << book.Ref << "," << endl;
    oss << "Title:\"" << book.Title << "\"," << endl;
    oss << "Author:\"" << book.Author << "\"," << endl;
    oss << "Stock:" << book.Stock << endl;
    oss << "}" << endl;
	return oss.str();
}

unsigned int JsonController::ToId(string value)
{
    istringstream iss(value);
	unsigned int id;
    iss >> id;
    return id;
}

void JsonController::ToObject(string value, Book& book)
{
    size_t posBegin, posEnd;

    posBegin = value.find("Ref:")+4;
    posEnd = value.find(",", posBegin);
    istringstream issId(value.substr(posBegin, posEnd-posBegin));
    issId >> book.Ref;

    posBegin = value.find("Title:")+7;
    posEnd = value.find(",", posBegin)-1;
    book.Title = value.substr(posBegin, posEnd-posBegin);

    posBegin = value.find("Author:")+8;
    posEnd = value.find(",", posBegin)-1;
    book.Author = value.substr(posBegin, posEnd-posBegin);

    posBegin = value.find("Stock:")+6;
    posEnd = value.find_first_of("\r\n", posBegin);
    istringstream issQte(value.substr(posBegin, posEnd-posBegin));
    issQte >> book.Stock;
}

TestTypeController::TestTypeController() : TestClass("TestTypeController", this), server(8003)
{
    addTest("GetList", &TestTypeController::GetList);
    addTest("GetObject", &TestTypeController::GetObject);
    addTest("CreateObject", &TestTypeController::CreateObject);
    addTest("ModifyObject", &TestTypeController::ModifyObject);
    addTest("DeleteObject", &TestTypeController::DeleteObject);

    server.AddRoute("/api/v1/books/[Id]", &myJsonCtrl);
    server.Start();
}

TestTypeController::~TestTypeController()
{
    server.Stop();
}

bool TestTypeController::GetList()
{
    string result;

    client.SendRequest("GET", "127.0.0.1", 8003, "/api/v1/books");
    assert(true==HttpHelper::WaitResponse(server, client));
    result = client.GetStatus();
    assert("200"==result);
    result = client.GetBody();
    assert("{ Books : ["==result.substr(0,11));
    return true;
}

bool TestTypeController::GetObject()
{
    string result;

    client.SendRequest("GET", "127.0.0.1", 8003, "/api/v1/books/1");
    assert(true==HttpHelper::WaitResponse(server, client));
    result = client.GetStatus();
    assert("200"==result);
    result = client.GetBody();
    assert("Ref:123342"==result.substr(result.find("Ref:"), 10));

    client.SendRequest("GET", "127.0.0.1", 8003, "/api/v1/books/8");
    assert(true==HttpHelper::WaitResponse(server, client));
    result = client.GetStatus();
    assert("404"==result);
    result = client.GetBody();
    assert("Book not found"==result);

    client.SendRequest("GET", "127.0.0.1", 8003, "/api/v1/books/999");
    assert(true==HttpHelper::WaitResponse(server, client));
    result = client.GetStatus();
    assert("403"==result);
    result = client.GetBody();
    assert("Id 999 unauthorized"==result);

    return true;
}

bool TestTypeController::CreateObject()
{
    Book myBook;
    Book readedBook;
    string result;


	myBook.Ref = 123654;
    myBook.Title = "Madame Bovary";
    myBook.Author = "Gustave Flaubert";
    myBook.Stock = 12;
    result = myJsonCtrl.ToString(myBook);
    client.SendRequest("POST", "127.0.0.1", 8003, "/api/v1/books", result);
    assert(true==HttpHelper::WaitResponse(server, client));
    result = client.GetBody();
    assert("Book ref 123654 already exist"==result);

    myBook.Ref = 123655;
    result = myJsonCtrl.ToString(myBook);
	client.SendRequest("POST", "127.0.0.1", 8003, "/api/v1/books", result);
    assert(true==HttpHelper::WaitResponse(server, client));
    result = client.GetBody();
    assert("New Id:5"==result);

    client.SendRequest("GET", "127.0.0.1", 8003, "/api/v1/books/5");
    assert(true==HttpHelper::WaitResponse(server, client));
    result = client.GetStatus();
    assert("200"==result);
    myJsonCtrl.ToObject(client.GetBody(), readedBook);
    assert(myBook.Ref==readedBook.Ref);
    assert(myBook.Title==readedBook.Title);
    assert(myBook.Author==readedBook.Author);
    assert(myBook.Stock==readedBook.Stock);

    return true;
}

bool TestTypeController::ModifyObject()
{
    Book readedBook;
    string result;
    string body;


	client.SendRequest("GET", "127.0.0.1", 8003, "/api/v1/books/2");
    assert(true==HttpHelper::WaitResponse(server, client));
    result = client.GetStatus();
    assert("200"==result);
    myJsonCtrl.ToObject(client.GetBody(), readedBook);

    readedBook.Stock = 71;
    body = myJsonCtrl.ToString(readedBook);

	client.SendRequest("PUT", "127.0.0.1", 8003, "/api/v1/books", body);
    assert(true==HttpHelper::WaitResponse(server, client));
    result = client.GetStatus();
    assert("500"==result);
    result = client.GetBody();
    assert("Id parameter is required"==result);

    client.SendRequest("PUT", "127.0.0.1", 8003, "/api/v1/books/9", body);
    assert(true==HttpHelper::WaitResponse(server, client));
    result = client.GetStatus();
    assert("404"==result);
    result = client.GetBody();
    assert("Book not found"==result);

    client.SendRequest("PUT", "127.0.0.1", 8003, "/api/v1/books/2", body);
    assert(true==HttpHelper::WaitResponse(server, client));
    result = client.GetStatus();
    assert("200"==result);
    result = client.GetBody();
    assert("OK"==result);

    return true;
}

bool TestTypeController::DeleteObject()
{
    string result;

    client.SendRequest("DELETE", "127.0.0.1", 8003, "/api/v1/books/7");
    assert(true==HttpHelper::WaitResponse(server, client));
    result = client.GetStatus();
    assert("404"==result);
    result = client.GetBody();
    assert("Book not found"==result);

    client.SendRequest("DELETE", "127.0.0.1", 8003, "/api/v1/books/5");
    assert(true==HttpHelper::WaitResponse(server, client));
    result = client.GetStatus();
    assert("200"==result);
    result = client.GetBody();
    assert("OK"==result);

    client.SendRequest("GET", "127.0.0.1", 8003, "/api/v1/books/5");
    assert(true==HttpHelper::WaitResponse(server, client));
    result = client.GetStatus();
    assert("404"==result);
    result = client.GetBody();
    assert("Book not found"==result);

    return true;
}
