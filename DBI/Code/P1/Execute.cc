#include "Execute.h"

//To actually run the commands, we need a lot of different moving pieces, lets bring all those in here
extern string gl_dbfile;
extern string gl_cat;
extern string gl_tpch;
extern char* deleteTable;
extern char* outFile;
extern struct Insert *insertToTab;
extern struct CreateTable *createTable;

extern string gl_tpch;
extern int mode;
bool Compiler::runOrPrint = false; 


// pass the Query planner tree and the pipe info to the execute object
void Execute::setrootNPipe(QPElement* _root,int nP){
        root=_root;
        numPipes = nP; 
}


//Initialize the execute object, its selfcontained, everything should already be set up before we hit this function
void Execute::init(){
    //make a local copy of the tablelist struct
    struct TableList *tempTable = tables;
    int totTables=0;
    //count the number of tables
    while(tempTable){
        tempTable=tempTable->next;
        totTables++;
    }

    //Create new pipes based on the information stored in setrootnpipe
    pipes = new Pipe*[numPipes+1];          
    for(int i=0;i<=numPipes;i++) pipes[i] = new Pipe(PIPE_BUFFER);

    //Create new DBfile objects as necessary
    dbfiles = new DBFile*[totTables];
    for(int i=0;i<totTables;i++) dbfiles[i]=new DBFile();

    //Do similarly for the rest too..
    selectfile= new class SelectFile*[totTables];
    for(int i=0;i<totTables;i++) selectfile[i]=new class SelectFile();
    currentDBFile = 0;
    
    selectpipe = new class SelectPipe*[totTables];
    for(int i=0;i<totTables;i++) selectpipe[i]=new class SelectPipe();
    selectPipes = 0;

    join = new class Join*[totTables];
    for(int i=0;i<totTables;i++) join[i]=new class Join();
    numjoin = 0;
    
    //only one instance of the ones below.
    groupby = new class GroupBy();

    project = new class Project();
    
    sum = new class Sum();
    
    dupremove = new DuplicateRemoval();

}

//Primary way to execute DDL querys
void Execute::executeDataQuery(){
    if(deleteTable != NULL){
        //For the Delete operation >>
        Catalog *c = cat;
        auto it = c->relToAttr.find(deleteTable);
        //check if table name exists in the DB already
        if(it!=c->relToAttr.end()){
            //if its , then erase it and write new catalog file
            cout<<"\n\nTable Found in DB. Tablename:"<<deleteTable<<endl;
            cat->relToAttr.erase(it);
            it =  cat->relToAttr.begin();
            ofstream out;
            out.open(gl_cat);
            //use this format to write to file
            while(it!=c->relToAttr.end()){
                out<<"\n";
                out<<"BEGIN";
                out<<"\n";

                out<<it->first.c_str();
                out<<"\n";
                out<<c->tableToFile[it->first].c_str();
                out<<"\n";
                int temp = it->second.size();
                for(int i=0;i<temp;i++)
                {
                    out<<it->second[i]->attr.c_str();
                    out<<" ";
                    //Notice the Endl at the end, it is very important to not mess with structure even though its fixed for us and our reader can ignore such typos.
                    //Good programming principles dictate not having to do such things.
                    out<<it->second[i]->type.c_str()<<endl;
                }
                
                out<<"END";
                out<<"\n";
                it++;
            }
            out.close();
            cout<<"Deleting bin file for "<<deleteTable<<" ..."<<endl;
            string filepath=string(gl_dbfile)+string(deleteTable)+".bin";
            char *temp = new char[filepath.length()];
            strcpy(temp,filepath.c_str());
            remove(temp);
            delete temp;
            cout<<"Delete of bin file successful."<<endl;
            cout<<"Deleting METAINF file for "<<deleteTable<<" ..."<<endl;
            filepath=string(gl_dbfile)+string(deleteTable)+".bin"+".METAINF";
            temp = new char[filepath.length()];
            strcpy(temp,filepath.c_str());
            remove(temp);
            delete temp;
            cout<<"Delete of METAINF file successful."<<endl;
            cat->attrToTable.clear();
            cat->attrToTable.clear();
            c->relToAttr.clear();
            cout<<"Start updating catalog file..."<<endl;
            c->init();
            cout<<"Catalog file entry for table deleted successfully."<<endl;
            cout<<"All Drop Ops Completed Successful!\n\n"<<endl;
        }else{
                cout<<"ERROR:\nTable Not Found! \n\tCannot find \""<<deleteTable<<"\" In DataBase\n\n"<<endl;
            return;
        }
    }
    else if(insertToTab != NULL){
         Catalog *c = cat;
        auto it = c->relToAttr.find(string(insertToTab->dbfile));
        if(it==c->relToAttr.end()){
            cout<<"\n\nERROR:\nTable Not Found! \n\tCannot find \""<<string(insertToTab->dbfile)<<"\" in DataBase\n\n"<<endl;
            // cout<<"error:Table Not Found! "<<string(insertToTab->dbfile)<<" In DataBase"<<endl;
            return;
        }

        cout<<"\n\nExisting Table found in DB."<<endl;
        cout<<"Attempting to open bin file..."<<endl;

        DBFile db;
        string fpath = string(gl_dbfile)+string(insertToTab->dbfile)+".bin";
        char *temp = new char[fpath.length()];
        strcpy(temp,fpath.c_str());
        db.Open(temp);
        cout<<"Opened bin file."<<endl;
        Schema s((char*)gl_cat.c_str(),insertToTab->dbfile);
        
        cout<<"Loading data from location: "<<string(gl_tpch)+string(insertToTab->filename)<<" ..."<<endl;
       
        fpath = string(gl_tpch)+string(insertToTab->filename);
        char* temptp = new char[fpath.length()];
        strcpy(temptp,fpath.c_str());
        cout<<"Inserting records now..."<<endl;
        db.Load(s,temptp);
        db.Close();
        db.Open(temp);
        int counter = 0;
        Record fetchme;
        db.MoveFirst();
        while (db.GetNext (fetchme) == 1) {
            counter += 1;
                //fetchme.Print (&s);
            //     if (counter % 10000 == 0) {
            //     cout << "Processed "<<counter << " records \n";
            // }
	    }
        cout<<"Processed "<<counter<<" records"<<endl;
        cout<<"All Insert Ops Successful!\n\n"<<endl;
        db.Close();
    }
    else if(createTable != NULL){
        cout<<"\n\nReading Catalog..."<<endl;
        Catalog* tempCat = cat->instantiate();
        attrType* catatts;
        cout<<"Adding new table information to the Catalog..."<<endl;
        char* tabToCreate = createTable->tableName;
        struct NameList* sortAtts = createTable->sortkeys;
        struct TableAtts* TabAtts = createTable->atts;
        string tableName(createTable->tableName);
        cat->tableToFile[tableName] = tableName + ".tbl";
        auto it = cat->relToAttr.find(tableName);
        if(it != cat->relToAttr.end()){
            // cout<<"Table already exists";
            cout<<"\n\nERROR:\nTable already exists! \n\t The tablename: \""<<tableName<<"\" is already present in DataBase.\n\t Please use something else.\n\n"<<endl;

            return;
        }

        cout<<"Adding the attributes information to the DB..."<<endl;
        while(TabAtts){
            struct CrAttr *op = TabAtts->Op;
            auto it = cat->attrToTable.find(string(op->value));
            if(it == cat->attrToTable.end()){
                TblLnkList *head=new TblLnkList(tableName);
                cat->attrToTable[string(op->value)]=head;
            }
            else{
                TblLnkList *node = new TblLnkList(tableName);
                node->next = cat->attrToTable[string(op->value)];
                cat->attrToTable[string(op->value)]=node;
            }            

            if(strcmp(op->type,"INTEGER") == 0){
            catatts = new attrType(string(op->value),string("Int"));
            cat->relToAttr[tableName].push_back(catatts);
            }
            else if(strcmp(op->type,"STRING")==0){
            catatts = new attrType(string(op->value),string("String"));
            cat->relToAttr[tableName].push_back(catatts);
            }
            else if(strcmp(op->type,"DOUBLE")==0){
                catatts = new attrType(string(op->value),string("Double"));
                cat->relToAttr[tableName].push_back(catatts);
            }
            TabAtts=TabAtts->next;
        }
        cout<<"All metainfo added successfully."<<endl;
        cout<<"Saving new table details to catalog file..."<<endl;
        it =  cat->relToAttr.begin();
        ofstream out;
        out.open(gl_cat);

        while(it!=cat->relToAttr.end())
        {
            out<<"\n"<<"BEGIN";
            out<<"\n"<<it->first;
            out<<"\n"<<cat->tableToFile[it->first];
            int len = it->second.size();
            for(int i=0;i<len;i++){
                out<<"\n"<<it->second[i]->attr<<" "<<it->second[i]->type;
            }
            out<<"\n"<<"END\n";
            it++;
        }
        cout<<"Catalog file updated successfully."<<endl;
        DBFile db;

         cout<<"Creating correct type of bin file..."<<endl;
        string filepath=string(gl_dbfile)+string(tableName)+".bin";
        if(createTable->sortkeys==NULL){
            // Heap Type
            char *temp = new char[filepath.length()];
            strcpy(temp,filepath.c_str());
            db.Create(temp,heap,NULL);
            cout<<"Heap type bin file created"<<endl;

        }
        else{
            //In case its sorted type, then bring in the ordermaker
            OrderMaker *om = new OrderMaker();
            int i = 0;
            struct NameList *sortatts = createTable->sortkeys;
            Schema *s = new Schema((char *)gl_cat.c_str(),(char*)tableName.c_str());
            while(sortatts!=NULL){
                int res = s->Find(sortatts->name);
                Type restype = s->FindType(sortatts->name);
                if(res != -1){
                    om->whichAtts[i] = res;
                    om->whichTypes[i] = restype;
                    i++;
                }
                sortatts = sortatts->next;
            }
            om->numAtts=i;
            int runlen=10;
            struct {
                OrderMaker *o; 
                int l;
            } startup = {om, runlen};
            char *temp = new char[filepath.length()];
            strcpy(temp,filepath.c_str());
            db.Create(temp,sorted,&startup);
            cout<<"Sort type bin file created"<<endl;
        }
        db.Close(); 
        cout<<"All Create Ops Successful!\n\n"<<endl;
    }
    else if(mode == 1){
        cout<<"\n\n Setting RunFlag to 1. \n\n"<<endl;
        Compiler::runOrPrint = true;
        Compiler::outFile = NULL;
    }
    else if(mode==2){
        cout<<"\n\n Setting RunFlag to 2\n\n"<<endl;
        Compiler::runOrPrint=true;
        Compiler::outFile = outFile;
    }
    else if(mode==3){
        cout<<"\n\n Setting RunFlag to 3\n\n"<<endl;
        Compiler::runOrPrint=false;
    }
}
        
void Execute::levelOrderPrint(QPElement* root){
    // queue<QPElement*> q;
    // q.push(root);
    // while(!q.empty()){
    //     QPElement* temp = q.front();
    //     q.pop();
    //     if(temp == NULL) cout<<"\nNULL\n";
    //     else{
    //         cout<<"Operation:"<<enumvals[temp->typeOps]<<endl;
    //         cout<<" In Pipe 1:"<<temp->inPipe1<<endl;
    //         cout<<" In Pipe 2:"<<temp->inPipe2<<endl;
    //         cout<<" OutPipe:"<<temp->outPipe<<endl;
    //         q.push(temp->left);
    //         q.push(temp->right);

    //     }

    // }

}
void Execute::printTree(QPElement* root){
    //printing inorder traversal of the root execution path
    if(root==NULL) {
        cout<<"\nNULL\n";
        return;
    }
    printTree(root->left);
    //traverse to left
   
    //print current root atts   
    cout<<endl;
    //Type of operation <Join,Select File,Select Pipe Etc...
    cout<<"Operation:"<<enumvals[root->typeOps]<<endl;
    cout<<" In Pipe 1:"<<root->inPipe1<<endl;
    cout<<" In Pipe 2:"<<root->inPipe2<<endl;
    cout<<" OutPipe:"<<root->outPipe<<endl;
    // Attribute *atts=root->outSchema->GetAtts();
    // int numatts=root->outSchema->GetNumAtts();
    // for(int i=0;i<numatts;i++){
    //     cout<<atts[i].name<<" "<<types[atts[i].myType]<<endl;
    // }
    // if(root->cnf!=NULL){
    //     //print CNF if present
    //     cout<<"\n CNF : ";
    //     root->cnf->Print();
    //     cout<<endl;
    // }   
    // if(root->om!=NULL)
    // {
    //     // Print order Maker if Necessary
    //     cout<<"\n OrderMaker : ";  
    //     root->om->Print();
    //     cout<<endl;
    // }
    // if(root->pAtts!=NULL){
    //     //Print attributes
    //     cout<<"\n Atts to keep : ";
    //     struct NameList *attstokeep=attsToSelect;
    //     while(attstokeep!=NULL)
    //     {
    //         cout<<" Att:"<<attstokeep->name<<endl;
    //         attstokeep=attstokeep->next;
    //     }
    // }
    // cout<<endl;
    //print right root atts
    
    printTree(root->right);
}

void Execute::executeQuery(QPElement *treeroot){
    if(treeroot == NULL) return;
    executeQuery(treeroot->right);
    executeQuery(treeroot->left);

    ///FIXME: DELETE IN END
    // cout<<"Operation:"<<enumvals[treeroot->typeOps]<<endl;
    // cout<<" In Pipe 1:"<<treeroot->inPipe1<<endl;
    // cout<<" In Pipe 2:"<<treeroot->inPipe2<<endl;
    // cout<<" OutPipe:"<<treeroot->outPipe<<endl;
    int ops;
    ops=treeroot->typeOps;
    if(ops==Project){         
        project->Use_n_Pages(10);
        int inputcnt=treeroot->pAtts->numAttsIn;
        int outputcnt=treeroot->pAtts->numAttsOut;
        int *ops=treeroot->pAtts->attsToKeep;
        project->Run(*pipes[treeroot->inPipe1],*pipes[treeroot->outPipe],ops,inputcnt,outputcnt);
        // project->WaitUntilDone();
    }

    else if(ops==GroupBy){                
        groupby->Use_n_Pages(10);
        groupby->Run(*pipes[treeroot->inPipe1],*pipes[treeroot->outPipe],*treeroot->om,*treeroot->func);
        // groupby->WaitUntilDone();
    }
    else if(ops==Sum){         
        sum->Use_n_Pages(10);
        sum->Run(*pipes[treeroot->inPipe1],*pipes[treeroot->outPipe],*treeroot->func);
        // sum->WaitUntilDone();
    }
    else if(ops==Join){         
        join[numjoin]->Use_n_Pages(10);
        join[numjoin]->Run(*pipes[treeroot->inPipe1],*pipes[treeroot->inPipe2],*pipes[treeroot->outPipe],*treeroot->cnf,*treeroot->tempRec);
        // join[numjoin]->WaitUntilDone();
        numjoin++;
    }
    else if(ops==Distinct){              
        dupremove->Use_n_Pages(10);
        dupremove->Run(*pipes[treeroot->inPipe1],*pipes[treeroot->outPipe],*treeroot->outSchema);
        // dupremove->WaitUntilDone();
    }
    else if (ops==SelectPipe){
        selectpipe[selectPipes]->Use_n_Pages(10);
        selectpipe[selectPipes]->Run(*pipes[treeroot->inPipe1],*pipes[treeroot->outPipe],*treeroot->cnf,*treeroot->tempRec);
        // selectpipe[selectPipes]->WaitUntilDone();
        selectPipes++;
    }
    else if(ops==SelectFile){
        selectfile[currentDBFile]->Use_n_Pages(10);
        string path=string(gl_dbfile);
        string filepath=path+treeroot->tblName+".bin";
        char *f = new char[filepath.length()];
        strcpy(f,filepath.c_str());                
        dbfiles[currentDBFile]->Open(f);
        selectfile[currentDBFile]->Run(*dbfiles[currentDBFile],*pipes[treeroot->outPipe],*treeroot->cnf,*treeroot->tempRec);
        // selectfile[currentDBFile]->WaitUntilDone();
        currentDBFile++;           
    }
    else{
        cout<<"\n Operation specified wrong"<<endl;
    }
}
void Execute::printNDel(){

    if(Compiler::outFile==NULL){
        Record rec;
        int rowcnt=0;
        while(pipes[numPipes - 1]->Remove(&rec)) {
            rec.Print(root->outSchema);
            rowcnt++;
        }
        cout<<"\nRecords returned:"<<rowcnt<<endl;
    }
    else
    {
        WriteOut w;
        FILE *f=fopen(Compiler::outFile,"w");
        w.Run(*pipes[numPipes - 1],f,*root->outSchema);
        w.WaitUntilDone();
        fclose(f);
    }

    for(int i=0;i<=numPipes;i++) delete pipes[i];

    for(int i=0;i<totTables;i++){

        dbfiles[i]->Close();
        delete dbfiles[i];
        delete selectfile[i];
        delete selectpipe[i];
        delete join[i];
    }
}