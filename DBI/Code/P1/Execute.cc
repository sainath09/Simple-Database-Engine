#include "Execute.h"

void Execute::printTree(QPElement* root){
    //printing inorder traversal of the node execution path
    if(root==NULL) return;
    //traverse to left
    printTree(root->left);
    //print current node atts   
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
    //print right node atts
    printTree(root->right);
}