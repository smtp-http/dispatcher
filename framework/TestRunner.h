#ifndef __TEST_RUNNER_H__
#define __TEST_RUNNER_H__


#include "TextTestResult.h"
#include "ExampleTestCase.h"


using namespace std;

typedef pair<string, Test *>           mapping;
typedef vector<pair<string, Test *> >   mappings;

class TestRunner
{
protected:
    bool                                m_wait;
    vector<pair<string,Test *> >        m_mappings;

public:
	            TestRunner    () : m_wait (false) {}
                ~TestRunner   ();

    void        run           (int ac, char **av);
    void        addTest       (string name, Test *test)
    { m_mappings.push_back (mapping (name, test)); }

protected:
    void        run (Test *test);
    void        printBanner ();

};




#endif
