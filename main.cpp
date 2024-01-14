#include <iostream>
#include <limits>
#include <string>
#include <chrono>
#include <ctime>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <windows.h>

using namespace std;

// &eroare in aceste funtii este pentru a contoriza erorile din flow
void clearInputs(); // elimina erorile inputului pentru urmatoarele inputuri
template <class CITIRE> CITIRE correctInput(string msg); // citeste si verifica inputul si l returneaza fara erori
template <class CITIRE> CITIRE correctInput(string msg, int &eroare);
bool correctInputYN(string msg); // citeste si verifica inputul de tip y/n si l returneaza fara erori, y = true, n = false
bool correctInputYN(string msg, int &eroare);
int correctInputNumber(string msg, int minim, int maxim); // citeste si verifica inputul de tip int si l returneaza daca este intre min si max
int correctInputNumber(string msg, int minim, int maxim, int &eroare);
void showValidSteps(); // List of all steps
void clearScreen(int time);

class Flow; // avem nevoie de declararea asta inainte pentru display step
class Step{
public:
    virtual bool execute(int &errors) = 0;
    virtual ~Step(){}
};
class TitleStep: public Step{
public:
    string title, subtitle;

    template <class CITIRE> friend CITIRE correctInput(string);

    bool execute(int &errors) override{
        title = correctInput<string>("Type the title: ", errors);
        subtitle = correctInput<string>("Type the subtitle: ", errors);
        return correctInputYN("Do you want to save this step? y/n: ", errors);
    }
};
class TextStep: public Step{
public:
    string title, copyText;

    bool execute(int &errors) override{
        title = correctInput<string>("Type the title: ", errors);
        copyText = correctInput<string>("Type the copy: ", errors);
        return correctInputYN("Do you want to save this step? y/n: ", errors);
    }
};
class TextInputStep: public Step{
public:
    string text;

    bool execute(int &errors) override{
        cout<<"Type all the text in one line!\n";
        text = correctInput<string>("Type the text you want in your file: ", errors);
        return correctInputYN("Do you want to save this step? y/n: ", errors);
    }
};
class NumberInputStep: public Step{
public:
    int numberInput;

    bool execute(int &errors) override{
        numberInput = correctInputNumber("Introduceti numarul dorit: ", INT_MIN, INT_MAX, errors);
        return correctInputYN("Do you want to save this number: " + to_string(numberInput) + "? y/n: ", errors);
    }
};
class CalculusStep: public Step{
public:
    float calculus;

    bool execute(int &errors) override{
        cout<<"Introduceti numarul initial, dupa simbolul, dupa al 2doilea numar din ecuatie."<<endl;
        cout<<"Se accepta numai numere intregi si simbolurile +,-,/,*,min,max."<<endl;
        NumberInputStep a, b;
        string symbol;
        if(a.execute(errors) == false)
            return false;
        while(true){
            symbol = correctInput<string>("Symbol: ", errors);
            if(symbol == "min" || symbol == "max" || symbol == "/" || symbol == "*" || symbol == "+" || symbol == "-")
                break;
            else
                if(correctInputYN("Symbol invald! Do you want to try again? y/n: ", errors) == false){
                    errors++;
                    return false;
                }
        }
        if(b.execute(errors) == false)
            return false;
        if(symbol == "+"){
            calculus = (float) a.numberInput + b.numberInput;
        }else if(symbol == "-"){
            calculus = (float) a.numberInput - b.numberInput;
        }else if(symbol == "*"){
            calculus = (float) a.numberInput * b.numberInput;
        }else if(symbol == "/"){
            calculus = (float) a.numberInput / b.numberInput;
        }else if(symbol == "min"){
            calculus = (float) min(a.numberInput, b.numberInput);
        }else if(symbol == "max"){
            calculus = (float) max(a.numberInput, b.numberInput);
        }
        cout<<"The output of "<<a.numberInput<<" "<<symbol<<" "<<b.numberInput<<" is "<<calculus<<"!"<<endl;
        return correctInputYN("Do you want to save this step? y/n: ", errors);
    }
};
class TextFileInputStep: public Step{
public:
    string fileName;
    ifstream fin;

    bool execute(int &errors) override{
        while(true){
            fileName = correctInput<string>("Enter the name of the file (no extension required): ", errors);
            fileName += ".txt";
            fin.open(fileName);
            if(!fin.is_open()){
                if(correctInputYN("File unable to open, do you want to try again? y/n: ", errors) == false)
                    return false;
            }else
                break;
        }
        return correctInputYN("Do you want to save this step? y/n: ", errors);
    }
};
class CsvFileInputStep: public Step{
public:
    string fileName;
    ifstream fin;

    bool execute(int &errors) override{
        while(true){
            fileName = correctInput<string>("Enter the name of the file (no extension required): ", errors);
            fileName += ".csv";
            fin.open(fileName);
            if(!fin.is_open()){
                if(correctInputYN("File unable to open, do you want to try again? y/n: ", errors) == false)
                    return false;
            }
        }
        return correctInputYN("Do you want to save this step? y/n: ", errors);
    }
};
class DisplayStep: public Step{ // shows last open file
public:
    string fileName;
    DisplayStep(Flow f);
    bool execute(int &errors) override{
        ifstream fin(fileName);
        if(fin.is_open()){ // daca exista un fisier de deschis
            if(correctInputYN("Doriti sa aflati continutul fisierului\"" + fileName + "\"? y/n: ", errors)){ // afisam continutul daca se vrea
                string line;
                while(getline(fin, line)){
                    cout<<line<<endl;
                }
            }
            return correctInputYN("Do you want to save this step? y/n: ", errors);
        }else{
            cout<<"Error! no file was opened!\n";
        }
        return false; // nu se salveaza step ul pentru ca nu s a gasit fisierul deschis
    }
};
class OutputStep: public Step{
public:
    string fileName, title, description;

    bool execute(int &errors) override{
        fileName = correctInput<string>("Enter the name of the file to be created (no extension required, it will be generated as .txt): ", errors);
        fileName += ".txt";
        ofstream fout(fileName);
        title = correctInput<string>("Enter the title of the file to be created (no extension required): ", errors);
        description = correctInput<string>("Enter the description of the file to be created (no extension required): ", errors);
        fout<<title<<endl<<description;
        return correctInputYN("Do you want to save this step? y/n: ", errors);
    }
};
class EndStep: public Step{
public:
    bool execute(int &errors) override{
        return correctInputYN("Do you want to finish the flow? y/n: ", errors);
    }
};

class Flow{
public:
    string name;
    vector<Step*> steps;

    string startTime; // YYYY-MM-DD HH:MM:SS
    string endTime;   // 0123456789012345678
    int skippedSteps;
    int errorScreens;

    Flow(){
        skippedSteps = 0;
        errorScreens = 0;
    }
    void addName(string newName){
        name = newName;
    }
    void setStartTime(){
        auto now = chrono::system_clock::now();
        auto now_c = chrono::system_clock::to_time_t(now);
        tm *tm_local = localtime(&now_c);
        ostringstream aux; //folosit pentru a convertii variabilia returnata de put_time in string
        aux << put_time(tm_local,"%Y-%m-%d %H:%M:%S");
        startTime = aux.str();
    }
    void setEndTime(){
        auto now = chrono::system_clock::now();
        auto now_c = chrono::system_clock::to_time_t(now);
        tm *tm_local = localtime(&now_c);
        ostringstream aux; //folosit pentru a convertii variabilia returnata de put_time in string
        aux << put_time(tm_local,"%Y-%m-%d %H:%M:%S");
        endTime = aux.str();
    }
    void saveFlow(){ /// unfinished
        vector<string> flows;
        string line1, line2;
        ifstream fin("flows.csv");
        vector<string> data;

        while(getline(fin, line1)){
            getline(fin, line2);
            data.push_back(line1);
            data.push_back(line2);
        }
        fin.close();

        ofstream fout("flows.csv");
        for(string line : data)
            fout<<line<<endl;

        fout<<name;
        for(Step* step: steps){
            fout<<",";
            if(dynamic_cast<TitleStep*>(step) != nullptr){
                fout<<"TITLE STEP";
            }else if(dynamic_cast<TextStep*>(step) != nullptr){
                fout<<"TEXT STEP";
            }else if(dynamic_cast<TextInputStep*>(step) != nullptr){
                fout<<"TEXT INPUT STEP";
            }else if(dynamic_cast<NumberInputStep*>(step) != nullptr){
                fout<<"NUMBER INPUT STEP";
            }else if(dynamic_cast<CalculusStep*>(step) != nullptr){
                fout<<"CALCULUS STEP";
            }else if(dynamic_cast<TextFileInputStep*>(step) != nullptr){
                fout<<"TEXT FILE INPUT STEP";
            }else if(dynamic_cast<CsvFileInputStep*>(step) != nullptr){
                fout<<"CSV FILE INPUT STEP";
            }else if(dynamic_cast<DisplayStep*>(step) != nullptr){
                fout<<"DISPLAY STEP";
            }else if(dynamic_cast<OutputStep*>(step) != nullptr){
                fout<<"OUTPUT STEP";
            }else if(dynamic_cast<EndStep*>(step) != nullptr){
                fout<<"END STEP";
            }else{
                fout<<"STEP";
            }
        }
        fout<<endl<<startTime<<","<<endTime<<","<<skippedSteps<<","<<errorScreens;
    }
};

DisplayStep::DisplayStep(Flow f){
    for(int k = f.steps.size() - 1; k >= 0; k--){ // trecem prin toti pasii pornind de la ultimul
        Step* step = f.steps[k];
        if(dynamic_cast<TextFileInputStep*>(step) != nullptr){ // daca gasim step de tip textFileInput sau csvFileInput
            TextFileInputStep* a = dynamic_cast<TextFileInputStep*>(step); // convertim step in textfilestep ca sa accesam membrii specifici
            fileName = a->fileName;
            break;
        }else if(dynamic_cast<CsvFileInputStep*>(step) != nullptr){
            CsvFileInputStep* a = dynamic_cast<CsvFileInputStep*>(step);
            fileName = a->fileName;
            break;
        }
    }
}

int workWithAFlowInterface(){
    cout<<"Introduceti un numar in functie de ce doriti sa realizati: \n";
    cout<<"1 - Creeaza un flow nou.\n";
    cout<<"2 - Deschideti un flow existent.\n";
    cout<<"3 - Eliminati un flow existent.\n";
    cout<<"4 - Iesiti din program.\n";
    return correctInputNumber("Input> ", 1, 4);
}
Flow createNewFlowInterface(bool &unselectedFlow){
        Flow createFlow;
        cout<<"Introduceti numele noului flow:\n";
        while(true){
            string flowName = correctInput<string>(">");
            if((flowName[0] >= 'a' && flowName[0] <= 'z')||(flowName[0] >= 'A' && flowName[0] <= 'Z')){
                flowName = flowName;
                if(false){
                    cout<<"Eroare! Acest flow este existent!\n";
                }else{
                    createFlow.addName(flowName);
                    break;
                }
            }else{
                cout<<"Nume invalid! Numele flow-ului trebuie sa inceapa cu o litera, incercati din nou!\n";
            }
        }
        createFlow.setStartTime();

        if(correctInputYN("Are you satisfied with the flow's name? y/n: "))
            unselectedFlow = false;
        return createFlow;
}
void openExistingFlowInterface(){
    bool exitWhile = false;
    while(!exitWhile){ // nu iesim din while pana nu intram intr-un flow sau renuntam
        string nameFlow = correctInput<string>("Type the name of the flow:\n>");

        string flowLine, analytics;
        bool flowFound = false;
        ifstream fin("flows.csv");
        vector<string> dataFlow;
        vector<string> dataAnalytics;
        getline(fin, flowLine); // first 2 lines dont have to be checked
        getline(fin, flowLine);

        while(getline(fin, flowLine)){
            getline(fin, analytics);
            stringstream ss(flowLine);
            vector<string> row;
            string field;
            while(getline(ss, field, ',')){
                row.push_back(field);
                if(row[0] == nameFlow){
                    dataFlow.push_back(field);
                }
            }
            if(row[0] == nameFlow){
                stringstream ss2(analytics);
                while(getline(ss2, field, ',')){
                    dataAnalytics.push_back(field);
                }
                flowFound = true;
                break;
            }
        }
        fin.close();

        if(flowFound){ // if we get the flow
            cout<<"Flow Name: "<<dataFlow[0]<<"\nSteps: ";
            int k = 0;
            for(string element: dataFlow){
                if(k >= 1) // k = 0 is the name of the flow
                    cout<<"\nStep "<<k<<": "<<dataFlow[k];
                k++;
            }
            cout<<"\nAnalytics for this flow:\n";
            cout<<"Started time: "<<dataAnalytics[0];
            cout<<"\nEnding time: "<<dataAnalytics[1];
            cout<<"\nSkipped steps: "<<dataAnalytics[2];
            cout<<"\nError screens: "<<dataAnalytics[3]<<endl<<endl;
            exitWhile = true;
        }else{
            if(correctInputYN("Flow inexistent! Doriti sa reveniti la meniul principal? y/n: "))
                exitWhile = true;
        }
    }
}
void deleteExistingFlow(){
    bool flowFound = false;
    while(!flowFound){ // nu iesim din while pana nu gasim flow-ul
        const string nameFlow = correctInput<string>("Type the name of the flow you want to delete:\n>");
        ifstream fin("flows.csv");

        vector<string> lines;
        string flowLine, analytics;
        getline(fin, flowLine);
        lines.push_back(flowLine);
        getline(fin, analytics);
        lines.push_back(analytics);

        while(getline(fin, flowLine)){
            getline(fin, analytics);
            string aux = flowLine;
            stringstream ss(aux);
            string field;

            getline(ss, field, ',');
            if(field != nameFlow){
                lines.push_back(flowLine);
                lines.push_back(analytics);
            }else{
                flowFound = true;
            }
        }
        fin.close();

        if(flowFound){ // daca am sters flow ul
            ofstream fout("flows.csv");
            for(string line: lines){
                fout<<line<<endl;
            }
            cout<<"Flow sters cu succes!";
            clearScreen(2000);
        }else{
            if(correctInputYN("Flow inexistent! Doriti sa reveniti la meniul principal? y/n: ")) // flow-ul ramane unselected in parametrii deci ne va intoare in meniul principal
                flowFound = true;
        }
    }
}
void doSelectedStep(int decizie, Flow &myFlow){
    Step * stepSelected;
    switch(decizie){
        case 0:{ // 0.END Step: Marks the end of a flow.
            stepSelected = new EndStep;
            myFlow.setEndTime();
            break;
        }
        case 1:{ // 1.TITLE Step: Contains title and subtitle (strings).
            stepSelected = new TitleStep;
            break;
        }
        case 2:{ // 2.TEXT Step: Contains title and copy (strings).
            stepSelected = new TextStep;
            break;
            }
        case 3:{ // 3.TEXT INPUT Step: Contains a description and a text input (strings).
            stepSelected = new TitleStep;
            break;
        }
        case 4:{ // 4.NUMBER INPUT Step: Contains a description and a number input (float).
            stepSelected = new NumberInputStep;
            break;
        }
        case 5:{ // 5.CALCULUS Step: Contains steps (integer) and operation (string).
            stepSelected = new CalculusStep;
            break;
        }
        case 6:{ // 6.DISPLAY Step: References a step (integer).
            stepSelected = new DisplayStep(myFlow);
            break;
        }
        case 7:{ // 7.TEXT FILE Input Step: Contains a description and a file name.
            stepSelected = new TextFileInputStep;
            break;
        }
        case 8:{ // 8.CSV FILE Input Step: Contains a description and a file name.
            stepSelected = new CsvFileInputStep;
            break;
        }
        case 9:{ // 9.OUTPUT Step: References a step (integer), includes file name, title, and description.
            stepSelected = new OutputStep;
            break;
        }
    }
    int errors = myFlow.errorScreens;
    if(stepSelected->execute(errors)){
        myFlow.steps.push_back(stepSelected);
        myFlow.errorScreens = errors;
    }else{
        myFlow.skippedSteps++;
        delete stepSelected;
    }
}

int main(){
    while(true){ // Acesta este while ul prin care funtioneaza programul ca o aplicatie
        Flow myFlow;
        bool unselectedFlow = true;
        while(unselectedFlow){ // Acesta este while- ul prin care alegem cum vrem sa lucram cu un flow
            int decizie1 = workWithAFlowInterface();
            clearScreen(500);
            if(decizie1 == 1){ // create new flow
                myFlow = createNewFlowInterface(unselectedFlow);
            }else if(decizie1 == 2){ // open and print existing flow
                openExistingFlowInterface();
            }else if(decizie1 == 3){ // delete existing flow
                deleteExistingFlow(); // o sa revenim imediat la meniul principal deoarece nu mai avem ce face cu un flow sters
            }else if(decizie1 == 4){ // exit program
                exit(0);
            }
        }
        clearScreen(500);
        while(true){ // Acesta este while- ul care ne pune sa adaugam pasi pana la chemarea pasului END
            if(myFlow.steps.size() > 0){ // daca s a efectuat minim un pas si ultimul pas este End terminam lucrul cu acest flow
                if(dynamic_cast<EndStep*>(myFlow.steps[myFlow.steps.size() - 1]) != nullptr){
                    break;
                }
            }
            if(correctInputYN("Doriti lista cu toti pasii pe care ii puteti face intr-un flow? y/n: "))
                showValidSteps();
            doSelectedStep(correctInputNumber("Select Step> ", 0, 9), myFlow);
            clearScreen(300);
        }
        myFlow.saveFlow();
    }
}

void clearInputs(){ // elimina erorile inputului pentru urmatoarele inputuri
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}
template <class CITIRE> CITIRE correctInput(string msg){ // citeste si verifica inputul si l returneaza fara erori
    cout<<msg;
    CITIRE input;

    try{
        if constexpr(is_same<CITIRE, string>::value) // if constexpr checks condition at compile time and is_same checks if CITIRE is a string
            getline(cin, input);
        else{
            cin>>input;
            clearInputs();
        }
    }catch(...){
        cout<<"Tip de date introdus gresit, incercati din nou: \n";
    }
    return input;
}
template <class CITIRE> CITIRE correctInput(string msg, int &eroare){ // citeste si verifica inputul si l returneaza fara erori
    cout<<msg;
    CITIRE input;

    try{
        if constexpr(is_same<CITIRE, string>::value) // if constexpr checks condition at compile time and is_same checks if CITIRE is a string
            getline(cin, input);
        else{
            cin>>input;
            clearInputs();
        }
    }catch(...){
        eroare++;
        cout<<"Tip de date introdus gresit, incercati din nou: \n";
    }
    return input;
}
bool correctInputYN(string msg){ // citeste si verifica inputul de tip y/n si l returneaza fara erori
    cout<<msg;
    string decizie;
    while(true){ // Nu iesim din while pana nu aflam output ul corect
        decizie = correctInput<string>("");
        if(decizie == "y" || decizie == "n") // daca introducem un input corect iesim
            break;
        else
            cout<<"Input incorect! Incercati din nou (y/n): ";
    }
    return decizie == "y";
}
bool correctInputYN(string msg, int &eroare){ // citeste si verifica inputul de tip y/n si l returneaza fara erori
    cout<<msg;
    string decizie;
    while(true){ // Nu iesim din while pana nu aflam output ul corect
        decizie = correctInput<string>("", eroare);
        if(decizie == "y" || decizie == "n") // daca introducem un input corect iesim
            break;
        else{
            eroare++;
            cout<<"Input incorect! Incercati din nou (y/n): ";
        }
    }
    return decizie == "y";
}
int correctInputNumber(string msg, int minim, int maxim){ // citeste si verifica inputul de tip int si l returneaza daca este intre min si max
    cout<<msg;
    int decizie;
    while(true){ // Nu iesim din while pana nu aflam output ul corect
        decizie = correctInput<int>("");
        if(minim <= decizie && decizie <= maxim) // daca introducem un input corect iesim
            break;
        else
            cout<<"Input Incorect! Incercati din nou: ";
    }
    return decizie;
}
int correctInputNumber(string msg, int minim, int maxim, int &eroare){ // citeste si verifica inputul de tip int si l returneaza daca este intre min si max
    cout<<msg;
    int decizie;
    while(true){ // Nu iesim din while pana nu aflam output ul corect
        decizie = correctInput<int>("", eroare);
        if(minim <= decizie && decizie <= maxim) // daca introducem un input corect iesim
            break;
        else{
            eroare++;
            cout<<"Input Incorect! Incercati din nou: ";
        }
    }
    return decizie;
}
void showValidSteps(){ // List of all steps
    cout<<"1.TITLE Step: Contains title and subtitle (strings).\n";
    cout<<"2.TEXT Step: Contains title and copy (strings).\n";
    cout<<"3.TEXT INPUT Step: Contains a description and a text input (strings).\n";
    cout<<"4.NUMBER INPUT Step: Contains a description and a number input (float).\n";
    cout<<"5.CALCULUS Step: Contains steps (integer) and operation (string).\n";
    cout<<"6.DISPLAY Step: References a step (integer).\n";
    cout<<"7.TEXT FILE Input Step: Contains a description and a file name.\n";
    cout<<"8.CSV FILE Input Step: Contains a description and a file name.\n";
    cout<<"9.OUTPUT Step: References a step (integer), includes file name, title, and description.\n";
    cout<<"0.END Step: Marks the end of a flow.\n";
}
void clearScreen(int time){
    Sleep(time);
    system("cls");
}

