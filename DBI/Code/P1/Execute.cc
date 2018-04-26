#include "Execute.h"
extern string gl_dbfile;
extern string gl_cat;
extern char* deleteTable;

void Execute::executeDataQuery(){
        if(deleteTable!=NULL){
            Catalog *c = cat;
            auto it = c->relToAttr.find(deleteTable);
            if(it!=c->relToAttr.end()){
                cat->relToAttr.erase(it);
                it =  cat->relToAttr.begin();
                ofstream out;
                out.open(gl_cat);
                while(it!=c->relToAttr.end())
                {
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
                        out<<it->second.at(i)->attr.c_str();
                        out<<" ";
                        out<<it->second.at(i)->type.c_str();
                    }
                    out<<"\n";
                    out<<"END";
                    it++;
                }
                out.close();

                // string filepath=string(binfilespath)+string(droptablename)+".bin";
                // char *tmp = new char[filepath.length()];
                // strcpy(tmp,filepath.c_str());
                // remove(tmp);
                // delete tmp;
                // filepath=string(binfilespath)+string(droptablename)+".bin"+".meta";
                // tmp = new char[filepath.length()];
                // strcpy(tmp,filepath.c_str());
                // remove(tmp);
                // delete tmp;
                
                // c->catalogAttribHash.clear();
                // c->catalogTableHash.clear();
                // c->relToAttr.clear();
                // c->initializeCatalogDS();
            }else{
                 cout<<"error:Table Not Found! "<<deleteTable<<" In DataBase:";
            return;
            }
        }
        
}

void Execute::printTree(QPElement* root){
    //printing inorder traversal of the root execution path
    if(root==NULL) return;
    //traverse to left
    printTree(root->left);
    //print current root atts   
    cout<<endl;
    //Type of operation <Join,Select File,Select Pipe Etc...
    cout<<"Operation:"<<enumvals[root->typeOps]<<endl;
    cout<<" In Pipe 1:"<<root->inPipe1<<endl;
    cout<<" In Pipe 2:"<<root->inPipe2<<endl;
    cout<<" OutPipe:"<<root->outPipe<<endl;
    Attribute *atts=root->outSchema->GetAtts();
    int numatts=root->outSchema->GetNumAtts();
    for(int i=0;i<numatts;i++){
        cout<<atts[i].name<<" "<<types[atts[i].myType]<<endl;
    }
    if(root->cnf!=NULL){
        //print CNF if present
        cout<<"\n CNF : ";
        root->cnf->Print();
        cout<<endl;
    }   
    if(root->om!=NULL)
    {
        // Print order Maker if Necessary
        cout<<"\n OrderMaker : ";  
        root->om->Print();
        cout<<endl;
    }
    if(root->pAtts!=NULL){
        //Print attributes
        cout<<"\n Atts to keep : ";
        struct NameList *attstokeep=attsToSelect;
        while(attstokeep!=NULL)
        {
            cout<<" Att:"<<attstokeep->name<<endl;
            attstokeep=attstokeep->next;
        }
    }
    cout<<endl;
    //print right root atts
    printTree(root->right);
}

void Execute::executeQuery(QPElement *treeroot){
    if(!treeroot) return;
    executeQuery(root->right);
    executeQuery(root->left);
    int ops;
    ops=treeroot->typeOps;
    if(ops==Project){         
        project->Use_n_Pages(10);
        int inputcnt=treeroot->pAtts->numAttsIn;
        int outputcnt=treeroot->pAtts->numAttsOut;
        int *ops=treeroot->pAtts->attsToKeep;
        project->Run(*pipes[treeroot->inPipe1],*pipes[treeroot->outPipe],ops,inputcnt,outputcnt);
    }

    else if(ops==GroupBy){                
        groupby->Use_n_Pages(10);
        groupby->Run(*pipes[treeroot->inPipe1],*pipes[treeroot->outPipe],*treeroot->om,*treeroot->func);
    }
    else if(ops==Sum){         
        sum->Use_n_Pages(10);
        sum->Run(*pipes[treeroot->inPipe1],*pipes[treeroot->outPipe],*treeroot->func);
    }
    else if(ops==Join){         
        join[numjoin]->Use_n_Pages(10);
        join[numjoin]->Run(*pipes[treeroot->inPipe1],*pipes[treeroot->inPipe2],*pipes[treeroot->outPipe],*treeroot->cnf,*treeroot->tempRec);
        numjoin++;
    }
    else if(ops==Distinct){              
        dupremove->Use_n_Pages(10);
        dupremove->Run(*pipes[treeroot->inPipe1],*pipes[treeroot->outPipe],*treeroot->outSchema);
    }
    else if (ops==SelectPipe){
        selectpipe[numPipes]->Use_n_Pages(10);
        selectpipe[selectPipes]->Run(*pipes[treeroot->inPipe1],*pipes[treeroot->outPipe],*treeroot->cnf,*treeroot->tempRec);
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
        currentDBFile++;                
    }
    else{
        cout<<"\n Operation specified wrong";
    }


    if(Compiler::outFile==NULL){
        Record rec;
        int rowcnt=0;
        while(pipes[numPipes]->Remove(&rec)) {
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