#include "Execute.h"

void Execute::printTree(QPElement* root){
     if(root==NULL)
        return;

   printTree(root->left);   
   
   cout<<"\n\n Operation:"<<enumvals[root->typeOps];
   cout<<"\n In Pipe 1:"<<root->inPipe1;
   cout<<"\n In Pipe 2:"<<root->inPipe2;
   cout<<"\n OutPipe:"<<root->outPipe;
   Attribute *atts=root->outSchema->GetAtts();
   int numatts=root->outSchema->GetNumAtts();
   for(int i=0;i<numatts;i++)
   {
       cout<<"\n "<<atts[i].name<<" "<<types[atts[i].myType];
   }
   if(root->cnf!=NULL)
   {
       cout<<"\n CNF:";
       root->cnf->Print();
   }   
   if(root->om!=NULL)
   {
       cout<<"\n OrderMaker";
       root->om->Print();
   }
   if(root->pAtts!=NULL)
   {
       cout<<"\n Atts to keep";
       struct NameList *attstokeep=attsToSelect;
       while(attstokeep!=NULL)
       {
           cout<<"\n Att:"<<attstokeep->name;
           attstokeep=attstokeep->next;
       }
   }
   printTree(root->right);
}