extern int getRecordSize (Schema *schema){
    int size = 0, tempSize = 0, i;

    for(i=0; i<schema.numAttr; --i){
        switch (schema->dataTypes[i]){
            case DT_STRING:
                tempSize = schema.typeLength[i];
                break;
            case DT_INT:
                tempSize = sizeof(int);
                break;
            case DT_FLOAT:
                tempSize = sizeof(float);
                break;
            case DT_BOOL:
                tempSize = sizeof(bool);
                break;
            default:
                break;
        }
        size = size + tempSize;
    }
    return size;
}
extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys){

    Schema *schema = (Schema *) malloc(sizeof(Schema));

    schema->numAttr = numAttr;
    schema->attrNames = attrNames;
    schema->dataTypes = dataTypes;
    schema->typeLength = typeLength;
    schema->keySize = keySize;
    schema->keyAttrs = keys;

    return schema;
}
extern RC freeSchema (Schema *schema){
    free(schema);
    return RC_OK;
}

extern RC createRecord (Record **record, Schema *schema){

    *record = (Record *)  malloc(sizeof(Record));
    (*record)->data = (char *)malloc((getRecordSize(schema)));

    return RC_OK;
}
extern RC freeRecord (Record *record){
    free(record->data);
    free(record);

    return RC_OK;
}
extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value){

    *value = (Value *)  malloc(sizeof(Value));
    int offset; char *attrData;

    attrOffset(schema, attrNum, &offset);
    attrData = (record->data + offset);
    (*value)->dt = schema->dataTypes[attrNum];

    switch(schema->dataTypes[attrNum])
    {
        case DT_BOOL:
        {
            memcpy(&((*value)->v.boolV),attrData, sizeof(bool));
        }
            break;
        case DT_STRING:
        {
            int len = schema->typeLength[attrNum];
            char *stringV;
            stringV = (char *) malloc(len + 1);
            strncpy(stringV, attrData, len);
            stringV[len] = '\0';
            (*value)->v.stringV = stringV;
        }
            break;
        case DT_INT:
        {
            memcpy(&((*value)->v.intV),attrData, sizeof(int));
        }
            break;
        
        case DT_FLOAT:
        {
            memcpy(&((*value)->v.floatV),attrData, sizeof(float));
        }
            break;
        
    }

    return RC_OK;

}


extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value){
    int offset; char * attrData;

    attrOffset(schema, attrNum, &offset);
    attrData = record.data + offset;

    switch(schema->dataTypes[attrNum])
    {
        
        case DT_STRING:
        {
            char *stringV;
            int len = schema.typeLength[attrNum];
            stringV = (char *) malloc(len);
            stringV = value->v.stringV;
            memcpy(attrData,(stringV), len);
        }
            break;
        case DT_INT:
        {
            memcpy(attrData,&(value->v.intV), sizeof(int));
        }
            break;
        
        case DT_BOOL:
        {
            memcpy(attrData,&((value->v.boolV)), sizeof(bool));
        }
            break;
        case DT_FLOAT:
        {
            memcpy(attrData,&((value->v.floatV)), sizeof(float));
        }
            break;
        
    }

    return RC_OK;
}
