//gcc <name of cpp file> -lstdc++ -lm
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cmath>
#include <vector>
#include <string>
#include <iomanip>
#include <queue>
#include <algorithm>

using namespace std;

// Глобальные переменные для системы
struct clients
{
    string name;
    tm timeIn;
    tm timeOut;
    int status;
    int tableNum;
    double cash;
    tm timeInTable;
};

struct tables
{
    int number;
    bool isFree = true;
};

struct fileInfo
{
    string clientName;
    int id;
    tm clientTime;
    int tableNumber = 0;
};

vector<clients> abonents;
vector<tables> places;
vector<fileInfo> inputEvents;
queue<clients> waitingAbonents;

tm timeStart;
tm timeEnd;

double price;
int countPlaces;

// Функция для вычисления времени, проведенного клиентом в компании
int calculateTimeSpent(const tm& startTime, const tm& endTime)
{
    time_t start_t = mktime(const_cast<tm*>(&startTime));
    time_t end_t = mktime(const_cast<tm*>(&endTime));
    double diff = difftime(end_t, start_t);
    return std::ceil(diff / 3600.0);
}

// Функция для проверки, попадает ли время в интервал рабочего дня
bool isWorkingHours(const tm& startTime, const tm& endTime, const tm& clientTime)
{
    tm start = startTime;
    tm end = endTime;
    tm client = clientTime;
    start.tm_year = client.tm_year;
    start.tm_mon = client.tm_mon;
    start.tm_mday = client.tm_mday;
    end.tm_year = client.tm_year;
    end.tm_mon = client.tm_mon;
    end.tm_mday = client.tm_mday;
    time_t start_t = mktime(&start);
    time_t end_t = mktime(&end);
    time_t client_t = mktime(&client);

    return (client_t >= start_t && client_t <= end_t);
}

// Функция для проверки расширения файла
bool isValidFileExtension(const string& filename)
{
    size_t pos = filename.find_last_of('.');
    if (pos == string::npos)
        return false;
    string extension = filename.substr(pos + 1);
    return extension == "txt";
}

// Функция для проверки формата считанного времени
bool isValidTime(const tm& time)
{
    return time.tm_hour >= 0 && time.tm_hour < 24 && time.tm_min >= 0 && time.tm_min < 60;
}

// Функция для преобразования времени
tm parseTime(const string& timeStr)
{
    tm time = {};
    istringstream ss(timeStr);
    ss >> get_time(&time, "%H:%M");
    return time;
}

void printTime(const tm& time)
{
    cout << std::setw(2) << std::setfill('0') << time.tm_hour << ":" << std::setw(2) << std::setfill('0') << time.tm_min << " ";
}

bool isAbonentInSystem(const fileInfo& note)
{
    bool isInSystem = false;
    for (auto iterC = abonents.begin(); iterC != abonents.end(); iterC++)
    {
        if (iterC->name == note.clientName)
        {
            isInSystem = true;
            break;
        }
    }

    if (isInSystem) // Проверка, что клиента в системе нет
    {
        return true;
    }

    return false;
}

void processingId1(const fileInfo& note) // Пришел
{
    if (!isWorkingHours(timeStart, timeEnd, note.clientTime)) // Проверка на рабочие часы
    {
        printTime(note.clientTime);
        cout << "13 NotOpenYet" << endl;
        return;
    }

    if (isAbonentInSystem(note)) // Проверка на наличие клиента в системе
    {
        printTime(note.clientTime);
        cout << "13 YouShallNotPass" << endl;
    }
    else // Добавление клиента в систему
    {
        clients tmpCLients;
        tmpCLients.name = note.clientName;
        tmpCLients.status = note.id;
        tmpCLients.timeIn = note.clientTime;
        tmpCLients.tableNum = note.tableNumber;
        abonents.push_back(tmpCLients);
    }
}

void processingId2(const fileInfo& note)
{
    for (auto iterTable = places.begin(); iterTable != places.end(); iterTable++) // Проверка на свободный стол
    {
        if (iterTable->number == note.tableNumber && iterTable->isFree == false)
        {
            printTime(note.clientTime);
            cout << "13 PlaceIsBusy" << endl;
        }
    }

    if (!isAbonentInSystem(note)) // Проверка на наличие клиента в системе
    {
        printTime(note.clientTime);
        cout << "13 ClientUnknown" << endl;
        return;
    }

    places[note.tableNumber - 1].isFree = false; // Занимает стол
    for (auto iterC = abonents.begin(); iterC != abonents.end(); iterC++) // Устанавливается время для подсчета выручки
    {
        if (iterC->name == note.clientName)
        {
            iterC->timeInTable = note.clientTime;
            iterC->status = note.id;
            iterC->tableNum = note.tableNumber;
            break;
        }
    }
}

void processingId3(const fileInfo& note)
{
    if (!isAbonentInSystem(note)) // Проверка на наличие клиента в системе
    {
        printTime(note.clientTime);
        cout << "13 ClientUnknown" << endl;
        return;
    }

    for (auto iterTable = places.begin(); iterTable != places.end(); iterTable++)
    {
        if (iterTable->isFree == true)
        {
            printTime(note.clientTime);
            cout << "13 ICanWaitNoLonger" << endl;
            return;
        }
    }

    if ((int)waitingAbonents.size() > countPlaces)
    {
        printTime(note.clientTime);
        cout << "11 " << note.clientName << endl;

        for (auto iterAb = abonents.begin(); iterAb != abonents.end(); iterAb++)
        {
            if (note.clientName == iterAb->name)
            {
                iterAb->timeOut = note.clientTime;
                iterAb->status = 11;
            }
        }

        return;
    }

    for (auto iterAb = abonents.begin(); iterAb != abonents.end(); iterAb++)
    {
        if (note.clientName == iterAb->name)
        {
            iterAb->status = note.id;
            clients tmpAb = *iterAb;
            waitingAbonents.push(tmpAb);
            break;
        }
    }
}

void processingId4(const fileInfo& note)
{
    if (!isAbonentInSystem(note)) // Проверка на наличие клиента в системе
    {
        printTime(note.clientTime);
        cout << "13 ClientUnknown" << endl;
        return;
    }

    int numberPlace = 0;
    for (auto iterAb = abonents.begin(); iterAb != abonents.end(); iterAb++)
    {
        if (iterAb->name == note.clientName)
        {
            iterAb->status = note.id;
            iterAb->timeOut = note.clientTime;
            numberPlace = iterAb->tableNum;
        }
    }

    places[numberPlace - 1].isFree = true;
    if (!waitingAbonents.empty())
    {
        clients tmpAb = waitingAbonents.front();
        for (auto iterAb = abonents.begin(); iterAb != abonents.end(); iterAb++)
        {
            if (iterAb->name == tmpAb.name)
            {
                iterAb->status = 12;
                iterAb->tableNum = numberPlace;
                places[numberPlace - 1].isFree = false;
                iterAb->timeInTable = note.clientTime;
                waitingAbonents.pop();
                printTime(note.clientTime);
                cout << "12 " << tmpAb.name << " " << iterAb->tableNum << endl;
            }
        }
    }
}

void diffTime(const tm& in, const tm& out, int& hour, int& minutes)
{
    time_t start_time = mktime(const_cast<tm*>(&in));
    time_t end_time = mktime(const_cast<tm*>(&out));
    double seconds = difftime(end_time, start_time);
    hour = static_cast<int>(seconds) / 3600;
    minutes = (static_cast<int>(seconds) % 3600) / 60;
}

void systemProcessing()
{
    for(auto iter = inputEvents.begin(); iter != inputEvents.end(); iter++)
    {
        fileInfo event = *iter;
        printTime(event.clientTime);
        cout << event.id << " " << event.clientName;
        if (event.tableNumber != 0)
        {
            cout << " " << event.tableNumber;
        }
        cout << endl;

        if (event.id == 1)
        {
            processingId1(event);
        }
        else if (event.id == 2)
        {
            processingId2(event);
        }
        else if (event.id == 3)
        {
            processingId3(event);
        }
        else if (event.id == 4)
        {
            processingId4(event);
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cerr << "Wrong Params!" << endl;
        return 1;
    }

    string filename = argv[1];
    if (!isValidFileExtension(filename))
    {
        cerr << "Invalid File Extension. File Must Be a .txt File." << endl;
        return 1;
    }

    ifstream file(filename);
    if (!file)
    {
        cerr << "Failed To Open File: " << filename << endl;
        return 1;
    }

    string line;
    if (!(getline(file, line)))
    {
        cerr << "Failed To Read First Line From File: " << filename << endl;
        return 1;
    }

    istringstream iss1(line);
    if (!(iss1 >> countPlaces))
    {
        cerr << "Invalid Format In File: " << filename << endl;
        return 1;
    }

    if (!(getline(file, line)))
    {
        cerr << "Failed To Read Second Line From File: " << filename << endl;
        return 1;
    }
    istringstream iss2(line);
    string start_time_str, end_time_str;
    if (!(iss2 >> start_time_str >> end_time_str))
    {
        cerr << "Invalid Format In File: " << filename << endl;
        return 1;
    }

    timeStart = parseTime(start_time_str);
    timeEnd = parseTime(end_time_str);
    if (!isValidTime(timeStart) || !isValidTime(timeEnd))
    {
        cerr << "Invalid Time Format In File: " << filename << endl;
        return 1;
    }

    if (!(getline(file, line)))
    {
        cerr << "Failed To Read Trird Line From File: " << filename << endl;
        return 1;
    }
    istringstream iss3(line);
    if (!(iss3 >> price))
    {
        cerr << "Invalid Format In File: " << filename << endl;
        return 1;
    }

    /*cout << countPlaces << " " << price << endl;
    cout << "TimeStart: " << std::setw(2) << std::setfill('0') << timeStart.tm_hour << ":"
                      << std::setw(2) << std::setfill('0') << timeStart.tm_min << std::endl;
    cout << "TimeEnd: " << std::setw(2) << std::setfill('0') << timeEnd.tm_hour << ":"
                      << std::setw(2) << std::setfill('0') << timeEnd.tm_min << std::endl;*/

    for (int i = 1; i <= countPlaces; i++)
    {
        tables tmpTab;
        tmpTab.number = i;
        places.push_back(tmpTab);
    }

    while (getline(file, line))
    {
        istringstream iss(line);
        fileInfo info;
        string time_str, name;
        if (!(iss >> time_str >> info.id >> name))
        {
            cerr << "Invalid Format In File: " << filename << endl;
            return 1;
        }
        info.clientTime = parseTime(time_str);
        info.clientName = name;
        if (iss >> info.tableNumber)
        {
            if (info.tableNumber <= 0 || info.tableNumber > countPlaces)
            {
                cerr << "Invalid Table Number In File: " << filename << endl;
                return 1;
            }
        }
        else
        {
            info.tableNumber = 0;
        }
        if (!isValidTime(info.clientTime))
        {
            cerr << "Invalid Time Format In File: " << filename << endl;
            return 1;
        }
        inputEvents.push_back(info);
    }
    printTime(timeStart);
    cout << endl;
    systemProcessing();
    vector<string> last;

    for (auto iterAb = abonents.begin(); iterAb != abonents.end(); iterAb++)
    {
        if (iterAb->status == 12 || iterAb->status == 2)
        {
            iterAb->timeOut = timeEnd;
        }
        if (iterAb->status != 4 && iterAb->status != 11)
        {
            last.push_back(iterAb->name);
        }
    }

    std::sort(last.begin(), last.end());
    for (auto iter = last.begin(); iter != last.end(); iter++)
    {
        printTime(timeEnd);
        cout << "11 " << *iter << endl;
    }
    printTime(timeEnd);
    cout << endl;

    for (auto iterAb = abonents.begin(); iterAb != abonents.end(); iterAb++)
    {
        if (iterAb->status == 4 || iterAb->status == 12 || iterAb->status == 2)
        {
            int tmpRes = calculateTimeSpent(iterAb->timeInTable, iterAb->timeOut);
            cout << iterAb->name << " " << (double)(tmpRes * price) << " ";
            int hours = 0, minutes = 0;
            diffTime(iterAb->timeInTable, iterAb->timeOut, hours, minutes);
            cout << std::setw(2) << std::setfill('0') << hours << ":"
                 << std::setw(2) << std::setfill('0') << minutes << std::endl;
        }
    }

    cout << endl << "List Of Clients Status: " << endl;
    for (auto iter = abonents.begin(); iter != abonents.end(); iter++)
    {
        cout << "Time In: ";
        printTime(iter->timeIn);
        cout << endl << "Name: " << iter->name << endl << "Status: " << iter->status << endl << "Start Time In Table: ";
        printTime(iter->timeInTable);
        cout << endl << "Time Out: ";
        printTime(iter->timeOut);
        cout << endl << endl;
    }

    return 0;
}
