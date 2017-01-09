//My key - 10d6133d-c30c-4e7b-b52f-6d5d479984cd
//Summoner verify - https://na.api.pvp.net/api/lol/na/v1.4/summoner/by-name/RiotSchmick?api_key=
//https://na.api.pvp.net/api/lol/na/v1.4/summoner/by-name/RiotSchmick?api_key=10d6133d-c30c-4e7b-b52f-6d5d479984cd

#include <cstdlib>
#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <sys/time.h>
#include <vector>
#include <fstream>
#include <map>
#include <windows.h>
#include <stdio.h>
#include <sys/stat.h>

using namespace std;

string key = "10d6133d-c30c-4e7b-b52f-6d5d479984cd";
string s_id;
string s_name;
int start_time = time(NULL);
string part_id;


struct MemoryStruct {
  char *memory;
  size_t size;
};

struct Game {
    string id;
    string champ;
    string team;
};

struct Item {
        string id;
        string name;
        int wins;
        int games;
        int avg_kills;
        int avg_deaths;
        int avg_assists;
        int avg_champ;
        int avg_total;
        int rel_champ;
        int rel_total;
        int avg_gold;
        int rel_gold;
};
    
struct Champion {
        string id;
        string name;
        int victories;
        int matches;
        string kda;
        int avg_champ;
        int avg_total;
        int rel_champ;
        int rel_total;
        vector<Item> items;
}; 

vector<Game> match_ids;
int rel_champ;
int rel_total;

vector<string> get_all_files_names_within_folder(string folder)
{ //http://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
    vector<string> names;
    char search_path[200];
    sprintf(search_path, "%s/*.*", folder.c_str());
    WIN32_FIND_DATA fd; 
    HANDLE hFind = ::FindFirstFile(search_path, &fd); 
    if(hFind != INVALID_HANDLE_VALUE) { 
        do { 
            // read all (real) files in current folder
            // , delete '!' read other 2 default folder . and ..
            if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
                names.push_back(fd.cFileName);
            }
        }while(::FindNextFile(hFind, &fd)); 
        ::FindClose(hFind); 
    } 
    return names;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

string getValue(string full, int &cursor, string first, int diff_one, string second, int diff_two)
{
    int pos, end;
    string result;   
    pos = full.find(first,cursor);
    pos += diff_one;
    end = full.find(second,pos);
    end += diff_two;
    result = full.substr(pos,end-pos);
    cursor = end -= diff_two;
    return result; 
}

string fileToString(string filename)
{    
    //TODO - Please help
    FILE* pFile = fopen(filename.c_str(),"r");
    if(pFile == NULL)
    {
        cout << "File does not exist, closing now" << endl;
        exit(0);
    }
    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    int size = ftell (pFile);
    rewind (pFile);

    char result_c [size];
    string end_result = "";
    while(fgets(result_c, size, pFile)) 
    {
         if (size == 1)
           break;
         string result_size = result_c;
         end_result.append(result_size);
         size -= result_size.size();
    }
    
    fclose(pFile);
    return end_result;
}

void updateFiles()
{
    //ERROR CHECK - check the version number first
    string result = fileToString("prefered.txt");
    int pos = result.find(",");
    pos = result.find(",",pos+1);
    string version = getValue(result,pos,",",1,",",0); 
    cerr << "Current version:" << version << endl;
    
    stringstream ss;
    cout << "Calling JSON item file.." << endl;
        //
        CURL *curl_handle;
        CURLcode res;
 
        struct MemoryStruct chunk;
 
        chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */ 
        chunk.size = 0;    /* no data at this point */ 
 
        curl_global_init(CURL_GLOBAL_ALL);     
        curl_handle = curl_easy_init();
        ss.str("");
        ss << "https://global.api.pvp.net/api/lol/static-data/na/v1.2/item?api_key=" << key;
        curl_easy_setopt(curl_handle, CURLOPT_URL, ss.str().c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        res = curl_easy_perform(curl_handle);
        if(res != CURLE_OK) {
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
                 curl_easy_strerror(res));
        }
        
        string results = chunk.memory;
        //ERROR CHECK - Check the version
        pos = 0;
        string pullversion = getValue(results,pos,"\"version\":",11,",",-1);
        if(pullversion != version)
        {
            FILE * prefFile;
            prefFile = fopen("prefered.txt","w");
            ss.str("");
            int first_comma = result.find(",");
            int end = result.find(",",first_comma+1);
            string file_begin = result.substr(0,end+1);
            ss << file_begin << pullversion << "," << '\0';
            fputs (ss.str().c_str(),prefFile);
            fclose (prefFile);
        }
        else
        {
           cout << "Files are up to date" << endl;
           return;   
        }
        
        //INFO - Part 1 - Generate Item List
        system("CLS");
        cout << "Generating item list to \"itemlist.txt\".." << endl;
        pos = 0;
        ss.str("");
        while((pos = results.find("\"id\":",pos)) != string::npos)
        {     //"id":0,"name":"",
              string key, name;
              pos--;
              key = getValue(results,pos,"\"id\":",5,",",0);
              name = getValue(results,pos,"\"name\":",8,",",-1);
              ss << key << "," << name << ",";
              cout << "Data located:" << key << "-" << name << endl;        
        }
        ss << '\0';
        
        system("CLS");
        cout << "Generating item list to \"itemlist.txt\".." << endl; 
        
         FILE * pFile;
        string filename = "itemset.txt";
        //DEBUG INFO - 
        cout << "Created file named:" << filename << endl;
        pFile = fopen(filename.c_str(),"w");
        if (pFile!=NULL)
        {
          fputs (ss.str().c_str(),pFile);
          fclose (pFile);
        }   
        
        //INFO - Part 2 - Generate the Champ List
        //TODO - Error check if request doesnt go through
        chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */ 
        chunk.size = 0;    /* no data at this point */ 
        ss.str("");
        ss << "https://global.api.pvp.net/api/lol/static-data/na/v1.2/champion?api_key=" << key;
        curl_easy_setopt(curl_handle, CURLOPT_URL, ss.str().c_str());
        res = curl_easy_perform(curl_handle);
        if(res != CURLE_OK) {
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
                 curl_easy_strerror(res));
        }
       
        results = chunk.memory;
        cout << "Generating champ list to \"champlist.txt\".." << endl;
        cout << "Generating item list to \"itemlist.txt\".." << endl; 
        cout << "This may take a moment.." << endl;

        ss.str("");
        pos = 0;
        while((pos = results.find("\"id\":",pos)) != string::npos)
        {     //"id":"Aatrox","key":"266",
              string key, name;
              key = getValue(results,pos,"\"id\":",5,",",0);
              name = getValue(results,pos,"\"key\":",7,",",-1);
              ss << key << "," << name << ",";
              cout << "Data located:" << key << "-" << name << endl;       
        }
        ss << '\0';
         
        filename = "champset.txt";
        pFile = fopen(filename.c_str(),"w");
        if (pFile!=NULL)
        {
          fputs (ss.str().c_str(),pFile);
          fclose (pFile);
        }   
                
        curl_easy_cleanup(curl_handle);
        free(chunk.memory); 
        curl_global_cleanup();
}

bool verifySummoner()
{
     CURL *curl_handle;
  CURLcode res;
 
  struct MemoryStruct chunk;
 
  chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */ 
  chunk.size = 0;    /* no data at this point */ 
 
  curl_global_init(CURL_GLOBAL_ALL);
  
    cout << "Hi! Welcome to the program!" << endl;
    cout << "Please input your Summoner Name:";
    getline(cin,s_name);
    
    //Need to remove the spaces from this
    string sanatized;
    for(int i=0;i<s_name.size();i++)
      if(s_name[i] != ' ')
        sanatized.push_back(s_name[i]);
        
    cout << "Sanatized s_name:" << sanatized << endl;

    cout << "Checking the validity of that name, one moment please..." << endl;
    
    stringstream ss;
    ss << "https://na.api.pvp.net/api/lol/na/v1.4/summoner/by-name/" << sanatized << "?api_key=" << key;
    //cerr << "Attempting site:" << ss.str() << endl; 
  /* init the curl session */ 
  curl_handle = curl_easy_init();
  /* specify URL to get */ 
  curl_easy_setopt(curl_handle, CURLOPT_URL, ss.str().c_str());
  /* send all data to this function  */ 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); 
  /* we pass our 'chunk' struct to the callback function */ 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk); 
  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */ 
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0"); 
  /* get it! */ 
  res = curl_easy_perform(curl_handle);
 
  /* check for errors */ 
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }
  else {
    //ERROR CHECK - Make sure rate limit has not been reached
        string error_check = chunk.memory;
        while(error_check.find("status_code") != string::npos)
        {//rate limit has been reached, wait 3 seconds then retry the previous request
            cout << "Overshot the rate limit, give it a second.." << endl;
            //int current_time = time(NULL);
            int end_time = time(NULL) + 3;
            while (time(NULL) < end_time)
            { /*Empty block to waste 3 seconds */ }
            chunk.memory = (char*)malloc(1);
            chunk.size = 0; 
            res = curl_easy_perform(curl_handle);
            error_check = chunk.memory;
        }
    //DEBUG INFO - 
    string results = chunk.memory;
    
    if(results.find("HTTP ERROR 404") != string::npos)
    {
       cout << "That summoner doesn't seem to exist.. Program will terminate now." << endl;
       system("PAUSE");
       exit(0);
    }
    //Assign the S_id
    int pos = results.find("\"id\":");
    int loc = results.find(",",pos);
    pos += 5;
    string id = results.substr(pos,loc-pos);
    cout << "Summoner id found:" << id << endl;
    s_id = id;
    FILE * pFile;
    pFile = fopen ("prefered.txt","w");
    stringstream ss;
    ss.str("");
    ss << s_name << "," << s_id << "," << "0.0" << "," << '\0';
    fputs(ss.str().c_str(),pFile);
    fclose(pFile);
    
  } 
  /* cleanup curl stuff */ 
  curl_easy_cleanup(curl_handle); 
  free(chunk.memory);
  /* we're done with libcurl, so clean it up */ 
  curl_global_cleanup();

  updateFiles();
}

void processData()
{
    /* REFERENCES:
       pFile -> result = Match file
       file -> c_result = Champ.txt
    */
    
    //TODO - Fill match_ids with what is currently in /Matches/
    cout << "Beginning to process the recent matches.." << endl;
    //Create a Champs directory
    CreateDirectory(".\\Champs", NULL);   
    //INFO - Read in the data
    for(int i=0;i<match_ids.size();i++)//For each match
    {
        
        cout << "Processing match:" << match_ids[i].id << endl;
        //First, open the correct file
        FILE * pFile;
        string filename = ".\\Matches\\match_" + match_ids[i].id + ".txt";
        pFile = fopen(filename.c_str(),"r");
        if (pFile!=NULL)//Does file exist?
        {
          cout << "File data sucessfully opened" << endl;
          fclose(pFile);
          string result = fileToString(filename);
          //ERROR CHECK - Is this a valid file?
          if (result.find("matchId") == string::npos)
          {
                cout << "Match data corrupted.. going to next file." << endl;
                cout << "Please send an error report to Andrew." << endl;
                system("PAUSE");
                continue;
          }
              
          //Find our player
          //INFO - This is what were looking for
          int pos =0;
          bool win;
          string item0, item1, item2, item3, item4, item5, item6;    
          string itemarray [] = { item0, item1, item2, item3, item4, item5, item6 };
          string kills, deaths, assists;
          string champ_damage, total_damage;
          string gold;
          int rel_gold;
         
          while(true) {
            if(match_ids[i].team != "300") {
              pos = result.find("teamId",pos);
              pos += 8;
              string snip = result.substr(pos,3);

              if(snip != match_ids[i].team) { //Check if were on the right team
                  pos++;
                  continue;
              }
            }
            pos = result.find("championId",pos);
            pos += 12;
            int end = result.find(",",pos);
            string snip = result.substr(pos,end-pos);

            if(snip != match_ids[i].champ) { //Check if its the right champ
                pos++;
                continue;
           }
            //At this point, weve found the right champ on the right team!
            //INFO - Finding the required data from this match now!
            //Now we locate their part_id, what items they ended with, and was it a win or loss 
            
            pos = result.find("winner",pos);
            pos += 8;
            snip = result.substr(pos,1);
            if(snip == "t")
              win = true;
            else
              win = false;

            for(int j=0;j<7;j++)
            {
               stringstream ss;
               ss << "item" << j;
               pos = result.find(ss.str(),pos);
               pos += 7;
               int end = result.find(",",pos);
               snip = result.substr(pos,end-pos);
               itemarray[j] = snip;
            }      
                          
            kills = getValue(result,pos,"kills",7,",",0);
            deaths = getValue(result,pos,"deaths",8,",",0);
            assists = getValue(result,pos,"assists",9,",",0);
            total_damage = getValue(result,pos,"totalDamageDealt",18,",",0);
            champ_damage = getValue(result,pos,"totalDamageDealtToChampions",29,",",0);
            gold = getValue(result,pos,"goldEarned",12,",",0);
            pos = result.find("participantId",pos);
            pos += 15;
            part_id = result.substr(pos,result.find(",",pos)-pos);        
               
            break;      
          }//while(true) - items and win results for that round have been found
          
          //TODO -
          //INFO - End of all champ specific stats, gathering global stats now
          pos = 0;
          rel_champ = 0;
          rel_total = 0;
          string check;
          int counter = 0;
          while ((result.find("\"winner\":",pos)) != string::npos) // calculate teams total damage
          {  
                if(counter == 10)
                  break;
                counter++;
                bool onteam = false;
                check = getValue(result,pos,"\"winner\":",9,",",0);
                if(check == "true" && win)
                {
                   onteam = true;
                }
                else if(check == "false" && win == false)
                { 
                    onteam = true;
                }
                if(onteam) {
                    string add_me;
                    add_me = getValue(result,pos,"totalDamageDealt",18,",",0);
                    rel_total += atoi(add_me.c_str());
                    add_me = getValue(result,pos,"totalDamageDealtToChampions",29,",",0);
                    rel_champ += atoi(add_me.c_str());
                    add_me = getValue(result,pos,"goldEarned",12,",",0);
                    rel_gold += atoi(add_me.c_str()); 
                }//if - this person is on your team
          }//while - calc teams total damage
          rel_champ = ((double)atoi(champ_damage.c_str()))/rel_champ*100;
          rel_total = ((double)atoi(total_damage.c_str()))/rel_total*100;
          rel_gold = ((double)atoi(gold.c_str()))/rel_gold*100;
          
          //Now its time to apply this data to the champ file
          //ERROR CHECK - Check if a champ file exists
          filename = ".\\Champs\\" + match_ids[i].champ + ".txt";
          cout << "checking for file:" << filename << endl;
          FILE *file;
          file = fopen(filename.c_str(), "r");
          string c_result;
          if (!(file))       
          {//File does not exist. create plz
             //INFO - Create a human-readable file specific for that champion
             cout << "Champ file does NOT exist, creating a new one.." << endl;
             c_result = fileToString("champset.txt"); //c_result == champset.txt
             
             int c_pos =0;
             //ERROR CHECK - If we search for ,20, and get ,200,... or 9 and ,69,
             string sanatized = "," + match_ids[i].champ + ",";
             c_pos = c_result.find(sanatized.c_str(), c_pos) + 1;
             int loc = c_pos + match_ids[i].champ.length();
 
             c_pos = c_result.find(",",c_pos);
             int c_end = c_result.find(",",c_pos+1);
             c_pos++;
             string champname = c_result.substr(c_pos,c_end-c_pos);
             cout << "Champ match found:" << champname << " - " << match_ids[i].champ << endl;
             
             stringstream ss;
             ss.str("");
             ss << match_ids[i].champ << "\n" << champname << ",\nVictories: 0,\nMatches: 0,\n" << "KDA: \nAverageChampDamage: \n" <<
                "AverageRelativeChampDamage: \n" << "AverageTotalDamage: \n" << "AverageRelativeTotalDamage: \n" << "AverageGold: \n" 
                << "AverageRelativeGold: \n" << "Item Data:\n\n" <<'\0';
             string contents = ss.str();
             //Put champ template into new file
             FILE *newfile;
             newfile = fopen(filename.c_str(), "w");
             fwrite (contents.c_str() , sizeof(char), contents.size(), newfile);
             fclose(newfile);               
          }//If - File does not alraedy exist      
          else
          {
             //Close the file, no template needed to be added
             fclose(file);
          } 
            cout << "Champ file exists, applying item data to it" << endl;
            //INFO - Now relating the recent match items to the champ file
            c_result = fileToString(filename);
            cout << "CURRENT STATE OF FILE (Before everything):" << c_result << endl;
            //INFO - Increment the champs games by one
            pos = 0;                
            pos = c_result.find("Matches:",pos);
            pos += 9;
            int end = c_result.find(",",pos);
            string s_wins = c_result.substr(pos,end-pos);
            int i_wins = atoi(s_wins.c_str());
            i_wins++;
            //erase the old number
            c_result.erase(pos,end-pos);
            stringstream ss;
            ss.str("");
            ss << i_wins;
            c_result.insert(pos,ss.str().c_str());
            
            //INFO - Increment the champs wins by one         
            pos = c_result.find("Victories:");
            pos += 11;
             end = c_result.find(",",pos);
             s_wins = c_result.substr(pos,end-pos);
             i_wins = atoi(s_wins.c_str());
            i_wins += win;
            //erase the old number
            c_result.erase(pos,end-pos);
            ss.str("");
            ss << i_wins;
            c_result.insert(pos,ss.str().c_str());
            
            //INFO - Add the new KDA to the file
            pos = c_result.find("KDA:",pos);
            pos = c_result.find("\n",pos);
            string kda_inject = kills + "/" + deaths + "/" + assists + ",";
            c_result.insert(pos,kda_inject.c_str());
            
            //INFO - Add the new AverageChampDamage to the file
            pos = c_result.find("AverageChampDamage:",pos);
            pos = c_result.find("\n",pos);
            string acd_inject = champ_damage + ",";
            c_result.insert(pos,acd_inject.c_str());
            
            //INFO - Add the new AverageRelativeChampDamage to the file
            pos = c_result.find("AverageRelativeChampDamage:",pos);
            pos = c_result.find("\n",pos);
            ss.str("");
            ss << rel_champ << ",";
            c_result.insert(pos,ss.str().c_str());
            
            //INFO - Add the new AverageTotalDamage to the file
            pos = c_result.find("AverageTotalDamage:",pos);
            pos = c_result.find("\n",pos);
            string atd_inject = total_damage + ",";
            c_result.insert(pos,atd_inject.c_str());
            
            //INFO - Add the new AverageRelativeTotalDamage to the file
            pos = c_result.find("AverageRelativeTotalDamage:",pos);
            pos = c_result.find("\n",pos);
            ss.str("");
            ss << rel_total << ",";
            c_result.insert(pos,ss.str().c_str());
            
            //INFO - AverageGold
            pos = c_result.find("AverageGold:",pos);
            pos = c_result.find("\n",pos);
            ss.str("");
            ss << gold << ",";
            c_result.insert(pos,ss.str().c_str());            
            //INFO - AverageRelativeGold
            pos = c_result.find("AverageRelativeGold:",pos);
            pos = c_result.find("\n",pos);
            ss.str("");
            ss << rel_gold << ",";
            c_result.insert(pos,ss.str().c_str()); 
                        
          //ERROR CHECK - If item dupes, make the rest of them 0
          for(int j=0;j<6;j++)
          {
             string check = itemarray[j];
             for(int k=j+1;k<7;k++)
               if(check == itemarray[k])
                  itemarray[k] = "0";    
          }
            
          //itemarray[0-6]
          //INFO - Update the 7 items acquired through the match to this champion
          for(int j=0;j<7; j++)
          {
             //ERROR CHECK - If item ID is 0
             if(itemarray[j] == "0")
               continue;
               
             cout << "Checking item id:" << itemarray[j] << endl;
             
             stringstream ss;
             
             //INFO - First, check if the champ has that item already
             int pos = 0;
             bool archived = false;
             while((pos = c_result.find("ItemID:",pos)) != string::npos) {
                cout << "ItemID:" << " located at " << pos << endl;
                cout << "Checking against existing template.." << endl;
                pos += 8;
                int end = c_result.find(",",pos);
                string item_id = c_result.substr(pos,end-pos);
                cout << "Does " << item_id << " == " << itemarray[j] << endl;
                if(item_id == itemarray[j])//Add one game, and win or loss
                {// INFO - MATCH
                    cout << "Item has been used before, updating stats.." << endl;
                   //INFO - Find the Wins value, then increment based on game results
                   pos = c_result.find("Wins:",pos);
                   pos += 6;
                   int end = c_result.find(",",pos);
                   string s_wins = c_result.substr(pos,end-pos);
                   int i_wins = atoi(s_wins.c_str());
                   i_wins += win;//add the win result, then insert it back into the file
                   //erase the old number
                   c_result.erase(pos,end-pos);
                   ss.str("");
                   ss << i_wins;
                   c_result.insert(pos,ss.str());
                   
                   //INFO - Find the total number of games, increment by one
                   //ALERT - RESUSING CODE FROM ABOVE, DONT MIND THE VARIABLE NAMES
                   pos = c_result.find("Games:",pos);
                   pos += 7;
                   end = c_result.find(",",pos);
                   s_wins = c_result.substr(pos,end-pos);
                   i_wins = atoi(s_wins.c_str());
                   i_wins++;
                   //erase the old number
                   c_result.erase(pos,end-pos);
                   ss.str("");
                   ss << i_wins;
                   c_result.insert(pos,ss.str().c_str());
                   
                   //INFO - Add another KDA to the array
                   pos = c_result.find("KDA:",pos);
                   pos = c_result.find("\n",pos);
                   string kda_inject = kills + "/" + deaths + "/" + assists + ",";
                   c_result.insert(pos,kda_inject.c_str());
                   cout << "Injected " << kda_inject << "into pos" << pos << endl;
                   
                  //INFO - Add AverageChampDamage to the array
                   pos = c_result.find("AverageChampDamage:",pos);
                   pos = c_result.find("\n",pos);
                   string acd_inject = champ_damage + ",";
                   c_result.insert(pos,acd_inject.c_str());
                   
                   //INFO - Add AverageRelativeChampDamage to the array
                   pos = c_result.find("AverageRelativeChampDamage:",pos);
                   pos = c_result.find("\n",pos);
                   ss.str("");
                   ss << rel_champ << ",";
                   c_result.insert(pos,ss.str().c_str());
                   
                   //INFO - Add AverageTotalDamage to the array
                   pos = c_result.find("AverageTotalDamage:",pos);
                   pos = c_result.find("\n",pos);
                   string atd_inject = total_damage + ",";
                   c_result.insert(pos,atd_inject.c_str());
                   
                   //INFO - Add AverageRelativeTotalDamage to the array
                   pos = c_result.find("AverageRelativeTotalDamage:",pos);
                   pos = c_result.find("\n",pos);
                   ss.str("");
                   ss << rel_total << ",";
                   c_result.insert(pos,ss.str().c_str());
                   
                   //INFO - Add AverageGold to the array
                   pos = c_result.find("AverageGold:",pos);
                   pos = c_result.find("\n",pos);
                   ss.str("");
                   ss << gold << ",";
                   c_result.insert(pos,ss.str().c_str()); 
                   
                   //INFO - Add AverageGold to the array
                   pos = c_result.find("AverageRelativeGold:",pos);
                   pos = c_result.find("\n",pos);
                   ss.str("");
                   ss << rel_gold << ",";
                   c_result.insert(pos,ss.str().c_str());              
                   
                   //The item has been updated!
                   archived = true;
                   break;                 
                }//If item already exists - Update it   
                pos = end;
                cout << "new cursor:" << pos << endl;
             }//While - check all current templates
             
             //INFO - If the item template does not exist, append it to the champ file
             if(archived == false)
             {
                cout << "Item has never been used before! Creating a template for it" << endl;
                string i_result = fileToString("itemset.txt");
                string finditem = "," + itemarray[j] + ",";
                int i_pos = i_result.find(finditem);
                i_pos = i_result.find(",",i_pos+1);
                i_pos++;
                int i_end = i_result.find(",",i_pos);
                string item_name = i_result.substr(i_pos,i_end-i_pos);
                //ERROR CHECK - If Item has been removed from game
                if(item_name == "")
                {  //item_name = "<ITEM REMOVED>";
                   cerr << "ITEM HAS BEEN REMOVED :<" << endl;
                   continue;
                }
                
                stringstream helpme;
                helpme << "===ITEM====\nItemID: " << itemarray[j] << ",\nName: " << item_name << ",\nWins: " << win << ",\nGames: 1" << ",\nKDA: " << kills 
                    << "/" << deaths << "/" << assists << ",\nAverageChampDamage: " << champ_damage << ",\nAverageRelativeChampDamage: " << rel_champ << 
                    ",\nAverageTotalDamage: " << total_damage << ",\nAverageRelativeTotalDamage: " << rel_total << ",\nAverageGold: " << gold << 
                    ",\nAverageRelativeGold: " << rel_gold<< ",\n" << '\0';
                cout << "Template created:" << helpme.str() << endl;
                c_result.append(helpme.str().c_str());
                
             }//If not archived already, create a template for it
          }//For each of the 7 items from the current match
        //Item and file successfully updated  
        
        //INFO - Rewrite the champ file with the updated c_result
        file = fopen(filename.c_str(),"w");
        fputs (c_result.c_str(),file);
        fclose (file);
        cout << "File " << filename << " successfully updated" << endl;       
      }//If - Match file successfully opened    
            
      //Move file to the Old directory when finished
      string matchold = ".\\Matches\\match_" + match_ids[i].id + ".txt";
      string matchnew = ".\\Matches\\Old\\match_" + match_ids[i].id + ".txt";
      
      rename(matchold.c_str(),matchnew.c_str());
      
      system("CLS");
      cout << "Progress:" << (double)i/match_ids.size()*100 << "%" << endl;
    }//for each match_ids
    
    //ERROR CHECK - Clean up mathch ids so no dupes you know
    //Clean match_ids
    int match_size = match_ids.size();
    for(int i=0;i<match_size;i++)
    {  
        match_ids.pop_back();
    }  
}

//Collect data on the 10 most recent games     
void collectData()
{
   //INFO - Write match data to a file
   //ERROR CHECK - Do we have to make a Matches folder?
   CreateDirectory(".\\Matches", NULL);
   CreateDirectory(".\\Matches\\Old", NULL);
  //https://na.api.pvp.net/api/lol/na/v1.3/game/by-summoner/43537151/recent?api_key=10d6133d-c30c-4e7b-b52f-6d5d479984cd
  cout << "Collecting data from the 10 most recent games.." << endl;
  CURL *curl_handle;
  CURLcode res;
 
  struct MemoryStruct chunk;
 
  chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */ 
  chunk.size = 0;    /* no data at this point */ 
 
  curl_global_init(CURL_GLOBAL_ALL);  
    
    stringstream ss;
    ss << "https://na.api.pvp.net/api/lol/na/v1.3/game/by-summoner/" << s_id << "/recent?api_key=" << key;
    cerr << "Attempting site:" << ss.str() << endl;
 
  curl_handle = curl_easy_init();
  curl_easy_setopt(curl_handle, CURLOPT_URL, ss.str().c_str());
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  res = curl_easy_perform(curl_handle);
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }
    //ERROR CHECK - Make sure rate limit has not been reached
    string error_check = chunk.memory;
    while(error_check.find("status_code") != string::npos)
    {//rate limit has been reached, wait 3 seconds then retry the previous request
        cout << "There was an error, retrying shortly.." << endl;
        int end_time = time(NULL) + 3;
        while (time(NULL) < end_time)
        { /*Empty block to waste 3 seconds */ }
        chunk.memory = (char*)malloc(1);
        chunk.size = 0;
        res = curl_easy_perform(curl_handle);
        error_check = chunk.memory;
    }
   string results = chunk.memory;
  //PARSING THE RECENT MATCHES FOR THEIR IDS
  cout << "Parsing through game date.." << endl;

  int pos =0;
  int loc, end;
  //INFO - Collect match IDs from the recent games
  while((loc = results.find("gameId", pos)) != string::npos)
  {
        string game_id, team_id, champ_id, game_type;
        //Get the game ID
        loc += 8;//Should give pos of the game id
        end = results.find(",",loc);
        game_id = results.substr(loc,end-loc);
        pos = end;
        //ERROR CHECK - Make sure its a NORMAL, RANKED_SOLO_5x5, RANKED_PREMADE_5x5, RANKED_TEAM_5x5, CAP_5x5
        loc = results.find("subType",pos);
        loc += 10;
        end = results.find(",",loc);
        game_type = results.substr(loc,end-loc-1);
        //ERROR CHECK
        if(!(game_type == "NORMAL" || game_type == "RANKED_SOLO_5x5" || game_type == "RANKED_PREMADE_5x5" || game_type == "RANKED_TEAM_5x5" || game_type == "CAP_5x5")) {
            cout << "Game found is not of valid type:" << game_type << "|" <<endl;
            continue;
        }
        pos = end;
        
        //Get the team ID
        loc = results.find("teamId", pos);
        loc += 8;//Should give pos of the game id
        end = results.find(",",loc);
        team_id = results.substr(loc,end-loc);
        pos = end;
        //Get the Champ ID
        loc = results.find("championId", pos);
        loc += 12;//Should give pos of the game id
        end = results.find(",",loc);
        champ_id = results.substr(loc,end-loc);
        pos = end;
        
        Game recent;
        recent.id = game_id;
        recent.champ = champ_id;
        recent.team = team_id;
        
        cout << "Game found:" << game_id << " " << champ_id << " " << team_id << endl;
        
        //ERROR CHECK - Check the Old folder for match_<matchid>
        string filename = ".\\Matches\\Old\\match_" + game_id + ".txt";
        FILE* mFile = fopen(filename.c_str(), "r");
        if(mFile) {
          fclose(mFile);
          cout << "File already used!" << endl;
          continue;
        }
        else
        {        
            match_ids.push_back(recent);
        }

  }
  cout << "Match IDs collected, collecting match data.." << endl;
  
  //INFO - Get the match data from the collected IDs
  for(int i=0;i<match_ids.size();i++)
  {
      //https://na.api.pvp.net/api/lol/na/v2.2/match/<1995842117>?includeTimeline=true&api_key=10d6133d-c30c-4e7b-b52f-6d5d479984cd
      //Clearn up, clean up
      ss.str("");
      chunk.memory = (char*)malloc(1); 
      chunk.size = 0;
  
      ss << "https://na.api.pvp.net/api/lol/na/v2.2/match/" << match_ids[i].id << "?includeTimeline=true&api_key=" << key;
      curl_easy_setopt(curl_handle, CURLOPT_URL, ss.str().c_str());
      res = curl_easy_perform(curl_handle);
      if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
          curl_easy_strerror(res));
      }
      else { 
        //ERROR CHECK - Make sure rate limit has not been reached
        string error_check = chunk.memory;
        while(error_check.find("status_code") != string::npos)
        {//rate limit has been reached, wait 3 seconds then retry the previous request
            cout << "Overshot the rate limit, give it a second.." << endl;
            //int current_time = time(NULL);
            int end_time = time(NULL) + 3;
            while (time(NULL) < end_time)
            { /*Empty block to waste 3 seconds */ }
            chunk.memory = (char*)malloc(1);
            chunk.size = 0; 
            res = curl_easy_perform(curl_handle);
            error_check = chunk.memory;
        }
        
        stringstream match_results;
        match_results << match_ids[i].team << "," << match_ids[i].champ << ",";
        match_results << chunk.memory;
        match_results << '\0';
        
        FILE * pFile;
        string filename = ".\\Matches\\match_" + match_ids[i].id + ".txt";
        //DEBUG INFO - 
        double progress = (double)i/match_ids.size();
        cout << "Created file named:" << filename << " | " << progress*100 << "%" << endl;
        pFile = fopen(filename.c_str(),"w");
        if (pFile!=NULL)
        {
          fputs (match_results.str().c_str(),pFile);
          fclose (pFile);
        }   
     }//else (successful curl request)
  }//for the last matches
  curl_easy_cleanup(curl_handle);
  free(chunk.memory); 
  curl_global_cleanup();

  return;
}

void importRanked()
{
   //INFO - Force Data collect
   collectData();
   processData();
   
   system("CLS");
   string answer;
   cout << "--Import Ranked Data--" << endl;
   cout << "1. Import Season 6 Data" << endl;
   cout << "2. Import All Ranked Data" << endl;
   cin >> answer;
   
   
   
   //INFO - Call ranked list
   CURL *curl_handle;
   CURLcode res;
 
   struct MemoryStruct chunk;
 
   chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */ 
   chunk.size = 0;    /* no data at this point */ 
 
   curl_global_init(CURL_GLOBAL_ALL);  
    
    stringstream ss;
    //https://na.api.pvp.net/api/lol/na/v2.2/matchlist/by-summoner/43537151?api_key=10d6133d-c30c-4e7b-b52f-6d5d479984cd
    ss << "https://na.api.pvp.net/api/lol/na/v2.2/matchlist/by-summoner/" << s_id << "?api_key=" << key;
    cerr << "Attempting site:" << ss.str() << endl;
 
   curl_handle = curl_easy_init();
   curl_easy_setopt(curl_handle, CURLOPT_URL, ss.str().c_str());
   curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
   curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
   curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
   res = curl_easy_perform(curl_handle);
   if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
   }
    //ERROR CHECK - Make sure rate limit has not been reached
    string error_check = chunk.memory;
    while(error_check.find("status_code") != string::npos)
    {//rate limit has been reached, wait 3 seconds then retry the previous request
        cout << "There was an error, retrying shortly.." << endl;
        int end_time = time(NULL) + 3;
        while (time(NULL) < end_time)
        { /*Empty block to waste 3 seconds */ }
        chunk.memory = (char*)malloc(1);
        chunk.size = 0;
        res = curl_easy_perform(curl_handle);
        error_check = chunk.memory;
    }
    string result = chunk.memory;
   
   
   //INFO - Gatether Match IDS
   int pos = 0;
   int iterator = 0;
   while((pos = result.find("\"matchId\":",pos)) != string::npos) {  
        string game_id = getValue(result,pos,"matchId",9,",",0);
        string champ_id = getValue(result,pos,"champion",10,",",0);
        string queue = getValue(result,pos,"queue",8,",",-1);
        string season = getValue(result,pos,"season",9,",",-1);
               
        Game recent;
        recent.id = game_id;
        recent.champ = champ_id;
        recent.team = "300";
        
        //INFO - if answer == "1" then only take this season data
        if(answer == "1")
        {
            if(season != "SEASON2016" && season != "PRESEASON2016")
              continue;
        }
        
        //ERROR CHECK - We dont want 3v3 games here >.>
        if(queue == "RANKED_TEAM_3x3")
          continue;
        
        //ERROR CHECK - Check the Old folder for match_<matchid>
        string filename = ".\\Matches\\Old\\match_" + game_id + ".txt";
        FILE* mFile = fopen(filename.c_str(), "r");
        if(mFile) {
          fclose(mFile);
          cout << "File already used! [" << game_id << "]" << endl;
          continue;
        }
        else
        {        
            match_ids.push_back(recent);
        }
        iterator++;
        cerr << "Games analyzed:" << iterator << endl;
    }//While - for each game on the ranked history list

   //INFO - Get the match data from the collected IDs
    for(int i=0;i<match_ids.size();i++)
    {
        //https://na.api.pvp.net/api/lol/na/v2.2/match/<1995842117>?includeTimeline=true&api_key=10d6133d-c30c-4e7b-b52f-6d5d479984cd
        //Clearn up, clean up
        ss.str("");
        chunk.memory = (char*)malloc(1); 
        chunk.size = 0;
    
        ss << "https://na.api.pvp.net/api/lol/na/v2.2/match/" << match_ids[i].id << "?includeTimeline=true&api_key=" << key;
        curl_easy_setopt(curl_handle, CURLOPT_URL, ss.str().c_str());
        res = curl_easy_perform(curl_handle);
        if(res != CURLE_OK) {
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
        }
        else { 
          //ERROR CHECK - Make sure rate limit has not been reached
          string error_check = chunk.memory;
          while(error_check.find("status_code") != string::npos)
          {//rate limit has been reached, wait 3 seconds then retry the previous request
              cout << "[Ranked] Overshot the rate limit, give it a second.." << endl;
              //int current_time = time(NULL);
              int end_time = time(NULL) + 3;
              while (time(NULL) < end_time)
              { /*Empty block to waste 3 seconds */ }
              chunk.memory = (char*)malloc(1);
              chunk.size = 0; 
              res = curl_easy_perform(curl_handle);
              error_check = chunk.memory;
          }
          
          stringstream match_results;
          match_results << match_ids[i].team << "," << match_ids[i].champ << ",";
          match_results << chunk.memory;
          match_results << '\0';
        
          FILE * pFile;
          string filename = ".\\Matches\\match_" + match_ids[i].id + ".txt";
          //DEBUG INFO - 
          double progress = (double)i/match_ids.size();
          cout << "Created file named:" << filename << " | " << progress*100 << "%" << endl;
          pFile = fopen(filename.c_str(),"w");
          if (pFile!=NULL)
          {
            fputs (match_results.str().c_str(),pFile);
            fclose (pFile);
          }   
       }//else (successful curl request)
    }//for the last 10 matches
    curl_easy_cleanup(curl_handle);
    free(chunk.memory); 
    curl_global_cleanup();
   
    processData(); 
 
    
}

int getAvg(string stats) //expect soemthing like 100,200,300,400,\n
{
    int total = 0;
    int i = 0;
    int pos = 0;
    while(stats[pos] != '\n')
    {
        int loc = stats.find(",",pos);
        string snip = stats.substr(pos,loc-pos);
        total += atoi(snip.c_str());
        pos = loc + 1;
        i++;   
    }
    total = total / i;
    return total;
}

void printItem(Item & item, int matches)
{
    string i_name = item.name;
    string i_id = item.id;
    int wins, games, i_kills, i_deaths, i_assists, i_avg_champ, i_rel_champ, i_avg_total, i_rel_total, i_avg_gold, i_rel_gold;
    wins = item.wins;
    games = item.games;
    i_kills = item.avg_kills;
    i_deaths = item.avg_deaths;
    i_assists = item.avg_assists;
    i_avg_champ = item.avg_champ;
    i_rel_champ = item.rel_champ;
    i_avg_total = item.avg_total;
    i_rel_total = item.rel_total;
    i_avg_gold = item.avg_gold;
    i_rel_gold = item.rel_gold;
                  
    cout << endl; //i_id string
    cout << "" << i_name << "[" << i_id << "]" << endl; 
    cout << "   Win Rate: " << ((double)wins/games)*100 << "% (" << wins << ")" << endl;
    cout << "   Popularity: " << ((double)games/matches)*100 << "% (" << games << ")" << endl;
    cout << "   Avg KDA: " << i_kills << "/" << i_deaths << "/" << i_assists << endl;
    cout << "   Avg Champ Dmg: " << i_avg_champ << endl;
    cout << "   Avg Rel Champ Dmg: " << i_rel_champ << "%" << endl;
    cout << "   Avg Total Dmg: " << i_avg_total << endl;
    cout << "   Avg Rel Total Dmg: " << i_rel_total << "%" << endl;
    cout << "   Avg Total Gold: " << i_avg_gold << endl;
    cout << "   Avg Rel Gold: " << i_rel_gold << "%" << endl;
}

void viewData()
{
    vector<string> files;
    files = get_all_files_names_within_folder("./Champs");
    //First, produce a nice list of champs to choose from
    map<string,string> champlist;
    for(int i=0;i<files.size();i++)
    {
        string dir = "./Champs/" + files[i];
        string result = ""; 
        result = fileToString(dir);
        int pos = result.find("\n");
        string id = result.substr(0,pos);
        pos += 1;
        string name = result.substr(pos,result.find(",")-pos);
        champlist[name] = id;
    }
    cout << "====Champions====" << endl;
    map<string,string>::const_iterator it;
    for(it = champlist.begin(); it != champlist.end(); it++)
    {
        string fill = it->second;
        if(fill.size() != 3)
          fill.push_back(' ');
        cout << fill << " - " << it->first << endl;
    }
    while(true) {
      cout << "Which champ would you like to look at? Please give the ID (Type \"X\" to exit, and \"C\" for champ list):";
      string choice;
      cin >> choice;
      FILE *checkFile;
      string filename = "./Champs/" + choice + ".txt";
      checkFile = fopen(filename.c_str(), "r");
      if (choice == "X" || choice == "x")
         break;
      else if (choice == "C" || choice == "c")
      {
            viewData();
            return;
      }
      else if(!(checkFile))
      {
         cout << "No data for that champ.. Try again. Type the letter X to exit." << endl;
         cout << "choice:" << choice << ":" << endl;   
      }
      else
      {
            //INFO - Champ has been chosen, and data is being displayed!
            fclose(checkFile);
            string result = fileToString(filename);
            string name = result.substr(result.find("\n") +1 ,result.find(",")-(result.find("\n")+1));
            int pos = 0;
            int victories = atoi(getValue(result,pos,"Victories:",11,",",0).c_str());
            int matches = atoi(getValue(result,pos,"Matches:",9,",",0).c_str());
            
            //INFO - Custom alg to calc the avg KDA
            pos = result.find("KDA:");
            pos += 5;
            int i = 0;
            int kills = 0;
            int deaths = 0;
            int assists = 0;
            while(result[pos] != '\n')
            {
                int loc = result.find("/",pos);
                string snip = result.substr(pos,loc-pos);
                kills += atoi(snip.c_str());
                pos = loc + 1;
                loc = result.find("/",pos);
                snip = result.substr(pos,loc-pos);
                deaths += atoi(snip.c_str());
                pos = loc + 1;
                loc = result.find(",",pos);
                snip = result.substr(pos,loc-pos);
                assists += atoi(snip.c_str());
                i++;
                pos = loc + 1;               
            }
            kills = kills / i;
            deaths = deaths / i;
            assists = assists / i;
            
            int avg_champ = 0;
            int avg_total = 0;
            int rel_champ = 0;
            int rel_total = 0;
            int avg_gold = 0;
            int rel_gold = 0;
           
            pos = result.find("AverageChampDamage:");
            pos += 20;
            int loc = result.find("\n",pos);
            string tofunc = result.substr(pos,loc-pos+1);
            avg_champ = getAvg(tofunc);
            
            pos = result.find("AverageRelativeChampDamage:");
            pos += 28;
            loc = result.find("\n",pos);
            tofunc = result.substr(pos,loc-pos+1);
            rel_champ = getAvg(tofunc);

            pos = result.find("AverageTotalDamage:");
            pos += 20;
            loc = result.find("\n",pos);
            tofunc = result.substr(pos,loc-pos+1);
            avg_total = getAvg(tofunc);
            
            pos = result.find("AverageRelativeTotalDamage:");
            pos += 28;
            loc = result.find("\n",pos);
            tofunc = result.substr(pos,loc-pos+1);
            rel_total = getAvg(tofunc);
            
            pos = result.find("AverageGold:");
            pos += 13;
            loc = result.find("\n",pos);
            tofunc = result.substr(pos,loc-pos+1);
            avg_gold = getAvg(tofunc);
            
            pos = result.find("AverageRelativeGold:");
            pos += 21;
            loc = result.find("\n",pos);
            tofunc = result.substr(pos,loc-pos+1);
            rel_gold = getAvg(tofunc);
                      
                        
            //INFO - Display Champ Data            
            system("CLS");
            cout << "====" << name << "====" << endl;
            cout << "Victories: " << victories << endl;
            cout << "Matches: " << matches << endl;
            cout << "Average KDA: " << kills << "/" << deaths << "/" << assists << endl;
            cout << "Average Champ Damage: " << avg_champ << endl;
            cout << "Avg Rel Champ Damage: " << rel_champ << "%" << endl;
            cout << "Average Total Damage: " << avg_total << endl;
            cout << "Avg Rel Total Damage: " << rel_total << "%" << endl;
            cout << "Average Gold: " << avg_gold << "g" <<  endl;
            cout << "Average Rel Gold: " << rel_gold << "%" << endl;
            cout << "\n====Item Data====" << endl;
            pos = result.find("Item Data");
            int help = 0;
           
            vector<Item> itemset;
            //INFO - Gather Item Data from champ file
            while((pos = result.find("ItemID:",pos)) != string::npos)//For each item
            {
                help++;
                //INFO - Gather the item stats first
                string i_id = getValue(result,pos,"ItemID:",8,",",0);
                string i_name = getValue(result,pos,"Name:",6,",",0);
                int wins = atoi(getValue(result,pos,"Wins:",6,",",0).c_str());    
                int games = atoi(getValue(result,pos,"Games:",7,",",0).c_str());
                
                //INFO - More custom KDA calculations
                int i = 0;
                int i_kills = 0;
                int i_deaths = 0;
                int i_assists = 0;
                pos = result.find("KDA:", pos);
                pos += 5;
                while(result[pos] != '\n')
                {    
                   int loc = result.find("/",pos);
                   string snip = result.substr(pos,loc-pos);
                   i_kills += atoi(snip.c_str());
                   pos = loc + 1;
                   loc = result.find("/",pos);
                   snip = result.substr(pos,loc-pos);
                   i_deaths += atoi(snip.c_str());
                   pos = loc + 1;
                   loc = result.find(",",pos);
                   snip = result.substr(pos,loc-pos);
                   i_assists += atoi(snip.c_str());
                   i++;
                   pos = loc + 1;               
               } 
               i_kills /= i;
               i_deaths /= i;
               i_assists /= i;
               
               int i_avg_champ = 0;
               int i_rel_champ = 0;
               int i_avg_total = 0;
               int i_rel_total = 0;
               int i_avg_gold = 0;
               int i_rel_gold = 0;
               
               pos = result.find("AverageChampDamage:",pos);
               pos += 20;
               int loc = result.find("\n",pos);
               string tofunc = result.substr(pos,loc-pos+1);
               i_avg_champ = getAvg(tofunc);
            
               pos = result.find("AverageRelativeChampDamage:",pos);
               pos += 28;
               loc = result.find("\n",pos);
               tofunc = result.substr(pos,loc-pos+1);
               i_rel_champ = getAvg(tofunc);
            
               pos = result.find("AverageTotalDamage:",pos);
               pos += 20;
               loc = result.find("\n",pos);
               tofunc = result.substr(pos,loc-pos+1);
               i_avg_total = getAvg(tofunc);
               
               pos = result.find("AverageRelativeTotalDamage:",pos);
               pos += 28;
               loc = result.find("\n",pos);
               tofunc = result.substr(pos,loc-pos+1);
               i_rel_total = getAvg(tofunc);
               
               pos = result.find("AverageGold:",pos);
               pos += 13;
               loc = result.find("\n",pos);
               tofunc = result.substr(pos,loc-pos+1);
               i_avg_gold = getAvg(tofunc);
               
               pos = result.find("AverageRelativeGold:",pos);
               pos += 21;
               loc = result.find("\n",pos);
               tofunc = result.substr(pos,loc-pos+1);
               i_rel_gold = getAvg(tofunc);
                   
               //INFO - Add to itemset vector             
               Item new_item;
               new_item.id = i_id;
               new_item.name = i_name;
               new_item.wins = wins;
               new_item.games = games;
               new_item.avg_kills = i_kills;
               new_item.avg_deaths = i_deaths;
               new_item.avg_assists = i_assists;
               new_item.avg_champ = i_avg_champ;
               new_item.avg_total = i_avg_total;
               new_item.rel_champ = i_rel_champ;
               new_item.rel_total = i_rel_total;
               new_item.avg_gold = i_avg_gold;
               new_item.rel_gold = i_rel_gold;
               
               itemset.push_back(new_item);  

               //TODO - In case something breaks :3        
               if(help > 500)
               {  
                  system("CLS");
                  cout << "Filestream corrupted, Please report this to Andrew." << endl;
                  system("PAUSE");
                  break;
               }
              
            }//While - for each item            
            
            int itemset_size = itemset.size();
            
            //INFO - Display actual item data
            cout << "Would you like to:" << endl;
            cout << "1. Print Top 10 Items" << endl;
            cout << "2. Print All Item Data" << endl;
            string answer;
            cin >> answer;
            if (answer == "1" || answer == "2")
            {               
                //INFO - Algorithm to decide most efficient items
                //((double)wins/games)*100
                //((double)games/matches)*100
                int page = 0;
                int pagenum = 1;
                
                int limit = 10;
                if (answer == "2")
                  limit = itemset.size();
                for(int i=0;i<limit;i++)
                {
                    double highest = 0;
                    int highest_index;
                    int highest_rel;
                    Item highest_item;
                    
                    for(int j=0;j<itemset.size();j++)
                    {
                        Item item = itemset[j];
                        //Here comes the algorithm
                        double goodness = (((double)item.wins/item.games)*100) + (((double)item.games/matches)*100) 
                           + item.rel_champ + item.rel_gold + item.avg_kills + item.avg_assists*.3 - item.avg_deaths/2;
                        if(highest < goodness)
                        {
                            highest = goodness;
                            highest_item = item;
                            highest_index = j;
                            highest_rel = item.rel_champ;
                        }
                        if(highest == goodness)
                        {
                            if(highest_rel < item.rel_champ)
                            {
                                 highest = goodness;
                                 highest_item = item;
                                 highest_index = j;
                                 highest_rel = item.rel_champ;
                            }   
                        }                                      
                    }
                    itemset.erase(itemset.begin() + highest_index); 
                    cout << endl << i << ".";
                    printItem(highest_item, matches);
                    
                    page++;
                    if(answer == "2")
                    {
                       if(page == 5)
                       {
                            cout << "-----End of Page " << pagenum << "/" << itemset_size/5 << "-----" << endl;
                            system("PAUSE");
                            page = 0;
                            pagenum++;
                        }   
                    }
                    //ERROR CHECK - If there are < 7 items  
                    if(itemset.size() == 0)
                      break;                  
                }
            }
      }//else - file discovered, printing champ data
    }//While(true) - refresh menu
}//Viewdata()

int main(void)
{
   bool previous = false;
   FILE *pFile;
   pFile = fopen("prefered.txt", "r");
   if(pFile != NULL)
     previous = true;
   if(previous == false) {
     fclose(pFile);
     cout << "QUICK NOTE: THIS PROGRAM WILL GENERATE A BUNCH OF TEXT FILES, PLEASE ENSURE ITS WITHIN A SPACE THAT YOU DONT MIND FILLING WITH TEXT FILES!" << endl;
     cout << "(Putting it in its own folder is probably a good idea!)\n" << endl;
     verifySummoner();
    }
    else
    {
        fclose(pFile);
        cout << "Getting user data from file.." << endl;
        string results = fileToString("prefered.txt");
        int loc = results.find(",");
        s_name = results.substr(0,loc);
        int end = results.find(",",loc+1);
        s_id = results.substr(loc+1,end-loc-1);
        cout << "Sname:" << s_name << " s_id:" << s_id << endl; 
        updateFiles();     
    }
    
    cout << "Hey there, " << s_name << ".\n" << endl;
    //Check if local data files exist
    //If so,
    while (true) {
        system("CLS");
        cout << "=====" << s_name << "=====" << endl;
        cout << "What would you like to do?" << endl;
        cout << "1. Force Data Collection" << endl;
        cout << "2. Review data" << endl;
        cout << "3. Adv/Dev Tools" << endl;
        cout << "4. Note from the Dev" << endl;
        cout << "5. Import Ranked Data (TAKES A LONG TIME)" << endl;
        cout << "Type anything else to exit" << endl;
        string answer;
        cin >> answer;

        if(answer == "1")//Force collect
        {
           collectData();
           processData();
        }
        else if(answer == "2")//Review time
        {
           viewData();
        }
        else if (answer == "3")//MORE TOOLS YAY
        {
           system("CLS");
           cout << "----Other Tools----" << endl;
           cout << "1. Update Item/Champ Lists (Program does this on start up)" << endl;
           cout << "2. Read in data from Matches folder (In case youre interupted mid processing)" << endl;
           cout << "3. Clear all of the datas (DANGER!!! :O!!)" << endl;
           cout << "Oh and btw 2. and 3. arent actually implemented so things will break if you use them :>" << endl;
           cin >> answer;
           if(answer == "1")
            {
                updateFiles(); 
            }
            else if(answer == "2")
            {
                vector<string> saveme;
                stringstream ss;
                
                saveme = get_all_files_names_within_folder("./Matches");
                for(int i=0;i<saveme.size();i++)
                {
                }
                
            }
            else if(answer == "3")
            {
                vector<string> killme;
                stringstream ss;
                //Clear Matches folder
                killme =  get_all_files_names_within_folder("./Matches/Old");
                for(int i=0;i<killme.size();i++)
                {  
                    
                   // DeleteFile(
                }
                //Clear matches old folder
                //Clear champs folder
                //Clear preferences            
            }   
        }
        else if(answer == "4")
        {
           cout << "Hiya! Andrew here. Thanks for helping me test this thing out, if you find any bugs then PLEASE tell me about it!" << endl;
           cout << "If you can figure out how to recreate it, please describe that to me as well." << endl;
           cout << "Also, if you have any kind of idea to improve this program, all suggestions are welcome!" << endl; 
           cout << "\nKNOWN BUGS:" << endl;
           cout << "- All clear, for now..." << endl;
           cout << endl;
           cout << "Thanks everyone ^^" << endl;
           system("PAUSE");
        }
        else if(answer == "5")
        {
            importRanked();
        }
        else
        {
          cout << "Thanks! Program now terminating.." << endl;
          system("PAUSE");
          exit(0);
        }
    }
    
    system("PAUSE");
    return EXIT_SUCCESS;
}
