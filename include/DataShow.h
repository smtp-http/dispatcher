
#include "InterfaceApi.h"

class DataShow : public IDataShowProvider
{	
public:
	virtual bool DataPrint(char *szFormat, ...);
	static DataShow& GetInstance();
};
