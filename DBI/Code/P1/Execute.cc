#include "Execute.h"
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
void Execute::setrootNPipe(QPElement* _root,int nP){
        root=_root;
        numPipes = nP; 
}

void Execute::init(){
    struct TableList *tempTable = tables;
        int totTables=0;
        while(tempTable){
            tempTable=tempTable->next;
            totTables++;
        }

        pipes = new Pipe*[numPipes+1];          
        for(int i=0;i<=numPipes;i++) pipes[i] = new Pipe(PIPE_BUFFER);

        dbfiles = new DBFile*[totTables];
        for(int i=0;i<totTables;i++) dbfiles[i]=new DBFile();

        selectfile= new class SelectFile*[totTables];
        for(int i=0;i<totTables;i++) selectfile[i]=new class SelectFile();
        currentDBFile = 0;
        
        selectpipe = new class SelectPipe*[totTables];
        for(int i=0;i<totTables;i++) selectpipe[i]=new class SelectPipe();
        selectPipes = 0;

        join = new class Join*[totTables];
        for(int i=0;i<totTables;i++) join[i]=new class Join();
        numjoin = 0;
        
        groupby = new class GroupBy();

        project = new class Project();
        
        sum = new class Sum();
        
        dupremove = new DuplicateRemoval();

}
void Execute::executeDataQuery(){
    if(deleteTable != NULL){
        Catalog *c = cat;
        auto it = c->relToAttr.find(deleteTable);
        if(it!=c->relToAttr.end()){
            cat->relToAttr.erase(it);
            it =  cat->relToAttr.begin();
            ofstream out;
            out.open(gl_cat);
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
                    out<<it->second[i]->type.c_str()<<endl;
                }
                out<<"\n";
                out<<"END";
                it++;
            }
            out.close();

            string filepath=string(gl_dbfile)+string(deleteTable)+".bin";
            char *temp = new char[filepath.length()];
            strcpy(temp,filepath.c_str());
            remove(temp);
            delete temp;
            filepath=string(gl_dbfile)+string(deleteTable)+".bin"+".METAINF";
            temp = new char[filepath.length()];
            strcpy(temp,filepath.c_str());
            remove(temp);
            delete temp;
            cat->attrToTable.clear();
            cat->attrToTable.clear();
            c->relToAttr.clear();
            c->init();
        }else{
                cout<<"error:Table Not Found! "<<deleteTable<<" In DataBase:";
            return;
        }
    }
    else if(insertToTab != NULL){
        DBFile db;
        string fpath = string(gl_dbfile)+string(insertToTab->dbfile)+".bin";
        char *temp = new char[fpath.length()];
        strcpy(temp,fpath.c_str());
        db.Open(temp);
        Schema s((char*)gl_cat.c_str(),insertToTab->dbfile);

        fpath = string(gl_tpch)+string(insertToTab->filename);
        char* temptp = new char[fpath.length()];
        strcpy(temptp,fpath.c_str());
        db.Load(s,temptp);
        db.Close();
        db.Open(temp);
        int counter = 0;
        Record fetchme;
        db.MoveFirst();
        while (db.GetNext (fetchme) == 1) {
            counter += 1;
                //fetchme.Print (&s);
                if (counter % 10000 == 0) {
                cout << counter << "\n";
            }
	    }
        cout<<counter<<endl;
        db.Close();
    }
    else if(createTable != NULL){
        Catalog* tempCat = cat->instantiate();
        attrType* catatts;
        char* tabToCreate = createTable->tableName;
        struct NameList* sortAtts = createTable->sortkeys;
        struct TableAtts* TabAtts = createTable->atts;
        string tableName(createTable->tableName);
        cat->tableToFile[tableName] = tableName + ".tbl";
        auto it = cat->relToAttr.find(tableName);
        if(it != cat->relToAttr.end()){
            cout<<"Table already exists";
            return;
        }
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
            //op->value; //FIXME: what does it do?
            TabAtts=TabAtts->next;
        }
    
        
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
            out<<"\n"<<"END";
            it++;
        }
        DBFile db;
        string filepath=string(gl_dbfile)+string(tableName)+".bin";
        if(createTable->sortkeys==NULL){
            // Heap Type
            char *temp = new char[filepath.length()];
            strcpy(temp,filepath.c_str());
            db.Create(temp,heap,NULL);
        }
        else{
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
        }
        db.Close(); 
    }
    else if(mode == 1){
        cout<<"\n setting RunFlag to 1";
        Compiler::runOrPrint = true;
        Compiler::outFile = NULL;
    }
    else if(mode==2){
        Compiler::runOrPrint=true;
        Compiler::outFile = outFile;
    }
    else if(mode==3){
        cout<<"\n setting RunFlag to 3";
        Compiler::runOrPrint=false;
    }
}
        
void Execute::levelOrderPrint(QPElement* root){
    queue<QPElement*> q;
    q.push(root);
    while(!q.empty()){
        QPElement* temp = q.front();
        q.pop();
        if(temp == NULL) cout<<"\nNULL\n";
        else{
            cout<<"Operation:"<<enumvals[temp->typeOps]<<endl;
            cout<<" In Pipe 1:"<<temp->inPipe1<<endl;
            cout<<" In Pipe 2:"<<temp->inPipe2<<endl;
            cout<<" OutPipe:"<<temp->outPipe<<endl;
            q.push(temp->left);
            q.push(temp->right);

        }

    }

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
    cout<<"Operation:"<<enumvals[treeroot->typeOps]<<endl;
    cout<<" In Pipe 1:"<<treeroot->inPipe1<<endl;
    cout<<" In Pipe 2:"<<treeroot->inPipe2<<endl;
    cout<<" OutPipe:"<<treeroot->outPipe<<endl;
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
        cout<<"\n Operation specified wrong";
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
        w.Run(*pipes[numPipes],f,*root->outSchema);
        w.WaitUntilDone();
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