#include <fstream>

#include <cstdlib>

#include <cppunit/config/SourcePrefix.h>
#include "GCoderTestCase.h"

#include "../Configuration.h"
#include "../GCoderOperation.h"
#include "../FileWriterOperation.h"
#include "../PathData.h"
#include "../GCodeEnvelope.h"

#include "mgl/abstractable.h"
#include "mgl/meshy.h"

using namespace std;
using namespace mgl;

CPPUNIT_TEST_SUITE_REGISTRATION( GCoderTestCase );

#define SINGLE_EXTRUDER_CONFIG "test_cases/GCoderTestCase/output/single_xtruder.config"

#define SINGLE_EXTRUDER_FILE_NAME "test_cases/GCoderTestCase/output/single_xtruder_warmup.gcode"
#define DUAL_EXTRUDER_FILE_NAME "test_cases/GCoderTestCase/output/dual_xtruder_warmup.gcode"
#define SINGLE_EXTRUDER_WITH_PATH "test_cases/GCoderTestCase/output/single_xtruder_with_path.gcode"
#define SINGLE_EXTRUDER_GRID_PATH "test_cases/GCoderTestCase/output/single_xtruder_grid_path.gcode"
#define SINGLE_EXTRUDER_MULTI_GRID_PATH "test_cases/GCoderTestCase/output/single_xtruder_multigrid_path.gcode"
#define SINGLE_EXTRUDER_KNOT "test_cases/GCoderTestCase/output/knot.gcode"

// for now, use cout, until we add Boost support
//#define BOOST_LOG_TRIVIAL(trace) cout
//#define BOOST_LOG_TRIVIAL(debug) cout
//#define BOOST_LOG_TRIVIAL(info) cout
//#define BOOST_LOG_TRIVIAL(warning) cout
//#define BOOST_LOG_TRIVIAL(error) cout
//#define BOOST_LOG_TRIVIAL(fatal) cout

void configurePlatform(Configuration& config, bool automaticBuildPlatform, double platformTemp )
{
	BOOST_LOG_TRIVIAL(trace)  << "Starting:" <<__FUNCTION__ << endl;
	config["scalingFactor"] = 1.0;
	config["platform"]["temperature"] = platformTemp;
	config["platform"]["automated"] = automaticBuildPlatform;
	config["platform"]["waitingPositionX"] = 52.0;
	config["platform"]["waitingPositionY"] = -57.0;
	config["platform"]["waitingPositionZ"] = 10.0;
	BOOST_LOG_TRIVIAL(trace)<< "Exiting:" <<__FUNCTION__ << endl;

}

// fills a configuration object with the data
// for a single extruder
void configureExtruder(Configuration& config, double temperature, double speed, double offsetX)
{
	Json::Value extruder;
	BOOST_LOG_TRIVIAL(trace)<< "Starting:" <<__FUNCTION__ << endl;
	extruder["leadIn"] = 0.25;
	extruder["leadOut"] = 0.35;
	extruder["defaultExtrusionSpeed"] = speed;
	extruder["extrusionTemperature"] = temperature;
	extruder["coordinateSystemOffsetX"] = offsetX;
	extruder["slowFeedRate"] = 1800;
	extruder["slowExtrusionSpeed"] = 4.47;
	extruder["fastFeedRate"] = 3000;
	extruder["fastExtrusionSpeed"] = 4.47;
	extruder["nozzleZ"] = 0.0;
	extruder["reversalExtrusionSpeed"] = 35.0;
	config["extruders"].append(extruder);
	BOOST_LOG_TRIVIAL(trace)<< "Exiting:" <<__FUNCTION__ << endl;
}

void configureSlicer(Configuration &config)
{
	config["slicer"]["firstLayerZ"] = 0.11;

	config["slicer"]["layerH"] = 0.35;
	config["slicer"]["layerW"] = 0.7;
	config["slicer"]["tubeSpacing"] = 0.8;
	config["slicer"]["angle"] = M_PI/2;

}
void configureSingleExtruder(Configuration &config)
{
	BOOST_LOG_TRIVIAL(trace)<< "Starting:" <<__FUNCTION__ << endl;
	configurePlatform(config, true, 110);
	configureExtruder(config, 220, 6, 0);
	BOOST_LOG_TRIVIAL(trace)<< "Exiting:" <<__FUNCTION__ << endl;
}

// fills a configuration object with data for 2 extruders
void configureDualExtruder(Configuration& config)
{
	BOOST_LOG_TRIVIAL(trace)<< "Starting:" <<__FUNCTION__ << endl;
	configurePlatform(config, true, 110)	;
	configureExtruder(config, 220, 6, 0);
	configureExtruder(config, 220, 6, 0);
	BOOST_LOG_TRIVIAL(trace)<< "Exiting:" <<__FUNCTION__ << endl;
}



void GCoderTestCase::setUp()
{
	BOOST_LOG_TRIVIAL(trace)<< " Starting:" <<__FUNCTION__ << endl;
	BOOST_LOG_TRIVIAL(trace)<< " Exiting:" <<__FUNCTION__ << endl;
}


void run_tool_chain(Configuration &config, vector<PathData*> &envelopes)
{

	BOOST_LOG_TRIVIAL(trace)<< "get Config static requirements:" <<__FUNCTION__ << endl;
	/// 1) (Optional)  call getStaticConfigRequirements, to make sure you can build a
	/// good configuration.
	Json::Value* gCoderRequires= GCoderOperation::getStaticConfigRequirements();
	Json::Value* fileWriterRequires= FileWriterOperation::getStaticConfigRequirements();
	assert( (void*)gCoderRequires != NULL);
	assert( (void*)fileWriterRequires != NULL);

	/// 2)  Build an instance of the object. This builds member functions, allocates much space etc
	BOOST_LOG_TRIVIAL(trace)<< "Build Operation Instances:" <<__FUNCTION__ << endl;
	GCoderOperation &tooler = *new GCoderOperation();
	FileWriterOperation &fileWriter = *new FileWriterOperation();

	///3) Create Output Vector(s) from each operation (not always required)
	BOOST_LOG_TRIVIAL(trace)<< "Build Output Vectors:" <<__FUNCTION__ << endl;
	vector<Operation*> empty;
	vector<Operation*> toolerOutputs;
	toolerOutputs.push_back(&fileWriter);

	///4) Build a Configure object. use the staticConfigRequirements to help you unless you know
	/// exactly what to build
	BOOST_LOG_TRIVIAL(trace)<< "Build Output Vectors:" <<__FUNCTION__ << endl;
	Configuration cfg;
	//	cfg.root["FileWriterOperation"]["prefix"] = Value("tester");
	//	cfg.root["FileWriterOperation"]["lang"] = Value("eng");


	///5) initalize the Object with your configuration, and your output list
	BOOST_LOG_TRIVIAL(trace)<< "Initalizing Operations:" <<__FUNCTION__ << endl;
	tooler.init(config,  toolerOutputs);
	fileWriter.init(config, empty);

	/// 6) Send a start signal to the first operation in the Operation Graph
	tooler.start();

	///7) Send inital one or more data envelopes to the object.
	for(std::vector<PathData*>::iterator it = envelopes.begin(); it != envelopes.end(); it++)
	{
		PathData *envelope = *it;
		BOOST_LOG_TRIVIAL(trace)<< "Accept Envelope @" << envelope <<" "<<__FUNCTION__ << endl;
		tooler.accept(*envelope);
	}

	/// 8) Send a finish signal to the first operation in the Operation Graph
	/// that call to finish will propagate down the graph automatically
	tooler.finish();

	//9) De-init (for safty)
	tooler.deinit();
	fileWriter.deinit();

	delete &tooler;
	delete &fileWriter;

	BOOST_LOG_TRIVIAL(trace)<< "Exiting:" <<__FUNCTION__ << endl;
}


void rectangle(Polygon& poly, double lower_x, double lower_y, double dx, double dy)
{
	Vector2 p0(lower_x, lower_y);
	Vector2 p1(p0.x, p0.y + dy);
	Vector2 p2(p1.x + dx, p1.y);
	Vector2 p3(p2.x, p2.y - dy);

	poly.push_back(p0);
	poly.push_back(p1);
	poly.push_back(p2);
	poly.push_back(p3);
	poly.push_back(p0);
}

// a function that adds 4 points to a polygon within the list paths for
// a new extruder.
void initSimplePath(PathData &d)
{
	BOOST_LOG_TRIVIAL(trace)<< "Starting:" <<__FUNCTION__ << endl;
	d.extruderSlices.push_back(ExtruderSlice());

	unsigned int last = d.extruderSlices.size() -1;
	Polygons &polys = d.extruderSlices[last].infills;


	for (int i=0; i< 4; i++)
	{
		polys.push_back(Polygon());
		size_t index= polys.size()-1;
		Polygon &poly = polys[index];

		double lower_x = -40 + 20 * i;
		double lower_y = -30;
		double dx = 10;
		double dy = 40;

		// randomize
		lower_x += 10.0 * ((double) rand()) / RAND_MAX;
		lower_y += 10.0 * ((double) rand()) / RAND_MAX;
		rectangle(poly, lower_x, lower_y, dx, dy);
	}
	BOOST_LOG_TRIVIAL(trace)<< "Exiting:" <<__FUNCTION__ << endl;
}

//
// This test creates a gcode file for single extruder machine
// The file contains code to home the tool and heat the extruder/platform
//
void GCoderTestCase::testSingleExtruder()
{
	BOOST_LOG_TRIVIAL(trace)<< "Starting:" <<__FUNCTION__ << endl;
	Configuration config;
	config["FileWriterOperation"]["filename"] = SINGLE_EXTRUDER_FILE_NAME;
	config["FileWriterOperation"]["format"]= ".gcode";

	configureSingleExtruder(config);
//	CPPUNIT_ASSERT_EQUAL((size_t)1, config.extruders.size());


	Json::StyledWriter w;
	string confstr = w.write(config.root);
	cout << confstr << endl;
	vector<PathData*> datas;
	run_tool_chain(config,datas );
	// verify that gcode file has been generated
	CPPUNIT_ASSERT( ifstream(SINGLE_EXTRUDER_FILE_NAME) );
	BOOST_LOG_TRIVIAL(trace)<< "Exiting:" <<__FUNCTION__ << endl;
}

//
// This test creates a gcode file for a dual extruder machine
//
void GCoderTestCase::testDualExtruders()
{
	BOOST_LOG_TRIVIAL(trace)<< "Starting:" <<__FUNCTION__ << endl;
	// cerate an empty configuration object
	Configuration config;
	// set the output fie
	config["FileWriterOperation"]["filename"]= DUAL_EXTRUDER_FILE_NAME;
	config["FileWriterOperation"]["format"]= ".gcode";

	// add extruder information
	configureDualExtruder(config);
//	CPPUNIT_ASSERT_EQUAL((size_t)2,config.extruders.size());
	// create a simple Gcode operation (no paths), initialize it and run it
	vector<PathData*> datas;
	run_tool_chain(config, datas);

	CPPUNIT_ASSERT( ifstream(DUAL_EXTRUDER_FILE_NAME) );

	BOOST_LOG_TRIVIAL(trace)<< "Exiting:" <<__FUNCTION__ << endl;
}


//
// 	This tests generates gcode for a simple rectangular path.
//
void GCoderTestCase::testSimplePath()
{
	BOOST_LOG_TRIVIAL(trace)<< "Starting:" <<__FUNCTION__ << endl;
	// create empty configuration and set the file name
	Configuration config;
	config["FileWriterOperation"]["filename"] = SINGLE_EXTRUDER_WITH_PATH;
	config["FileWriterOperation"]["format"]= ".gcode";

	// load 1 extruder
	configureSingleExtruder(config);
	//	CPPUNIT_ASSERT_EQUAL((size_t)1, config.extruders.size());
	// create a path message as if received by a pather operation

	PathData *path = new PathData(0.2);
	// add a simple rectangular path for the single extruder
	initSimplePath(*path);
	std::vector<PathData*> datas;
	datas.push_back( (PathData*) path);
	// instaniate a gcoder and send it the path as an envelope.
	run_tool_chain(config, datas);

	// cleanup the data
	for(std::vector<PathData*>::iterator it = datas.begin(); it != datas.end(); it++)
	{
		PathData* data = *it;
		//data->release();
	}

	// verify that gcode file has been generated
	CPPUNIT_ASSERT( ifstream(SINGLE_EXTRUDER_WITH_PATH) );
	BOOST_LOG_TRIVIAL(trace)<< "Exiting:" <<__FUNCTION__ << endl;
}

void GCoderTestCase::testConfig()
{
	BOOST_LOG_TRIVIAL(trace)<< "Starting:" <<__FUNCTION__ << endl;

	Configuration conf;
	std::string p = conf.root["programName"].asString();
	cout << endl << endl << endl << "PROGRAM NAME: " << p << endl;
	CPPUNIT_ASSERT(p == "Miracle-Grue");

	configureSingleExtruder(conf);
	Json::StyledWriter w;
	string confstr = w.write(conf.root);
	cout << confstr << endl;

	CPPUNIT_ASSERT(conf.root["extruders"].isArray());
	CPPUNIT_ASSERT(conf.root["extruders"].isValidIndex(0));

	cout << "ExtruderCount " << conf.root["extruders"].size() << endl;
	GCoder single;
	single.loadData(conf);

	cout << endl << endl << endl << "READ!" << endl;

	CPPUNIT_ASSERT(single.readExtruders().size() ==1);

	// save config for single extruder
	Configuration config;
	configureSingleExtruder(config);
	configureSlicer(config);
	string s = config.asJson();

	ofstream outfile;
	outfile.open(SINGLE_EXTRUDER_CONFIG);
	outfile << s;
	outfile.close();

	Configuration phoenix;
	phoenix.readFromFile(SINGLE_EXTRUDER_CONFIG);
	cout << "phoenix:" << phoenix.root["programName"].asString() << endl;
	CPPUNIT_ASSERT(phoenix.root["programName"] == "Miracle-Grue");

	BOOST_LOG_TRIVIAL(trace)<< "Exiting:" <<__FUNCTION__ << endl;
}

void gcodeStreamFormat(ostream &ss)
{
    try
    {
    	ss.imbue(std::locale("en_US.UTF-8"));
    }
    catch(...)
    {
    	ss.imbue(std::locale("C"));
    }
    ss.setf(ios_base::floatfield, ios::floatfield);            // floatfield not set
	ss.precision(4);
}


void GCoderTestCase::testFloatFormat()
{
	stringstream ss;
	gcodeStreamFormat(ss);

	ss << endl;
	ss << "loc: " << ss.getloc().name() << endl;

	CPPUNIT_ASSERT_EQUAL(ss.getloc().name(), string("en_US.UTF-8")); // std::string("C") );

	//locale myloc(  locale(),    // C++ default locale
    //       new WithComma);// Own numeric facet
	ss << endl;
	// ss << "LOCALE name: " << myloc.name() << endl;
	ss << "num: " << 3.1415927 << endl;
	cout << ss.str() << endl;
	BOOST_LOG_TRIVIAL(trace)<< "Exiting:" <<__FUNCTION__ << endl;

}


void initHorizontalGridPath(PathData &d, double lowerX, double lowerY, double dx, double dy, int lineCount)
{
	d.extruderSlices.push_back(ExtruderSlice());
	Polygons &polys = d.extruderSlices[0].infills;

	bool flip = false;
	for (int i=0; i< lineCount; i++)
	{
		polys.push_back(Polygon());
		size_t index= polys.size()-1;
		Polygon &poly = polys[index];

		double y = lowerY + i * dy / lineCount;
		Vector2 p0 (lowerX, y);
		Vector2 p1 (p0.x + dx, y );
		if(!flip)
		{
			poly.push_back(p0);
			poly.push_back(p1);
		}
		else
		{
			poly.push_back(p1);
			poly.push_back(p0);
		}
		flip = !flip;
	}
}

void initVerticalGridPath(PathData &d, double lowerX, double lowerY, double dx, double dy, int lineCount)
{
	d.extruderSlices.push_back(ExtruderSlice());
	Polygons &polys = d.extruderSlices[0].infills;

	bool flip = false;

	for (int i=0; i< lineCount; i++)
	{
		polys.push_back(Polygon());
		size_t index= polys.size()-1;
		Polygon &poly = polys[index];

		double x = lowerX + i * dx / lineCount;
		Vector2 p0 (x, lowerY);
		Vector2 p1 (x, lowerY + dy );
		if(!flip)
		{
			poly.push_back(p0);
			poly.push_back(p1);
		}
		else
		{
			poly.push_back(p1);
			poly.push_back(p0);
		}
		flip = !flip;
	}
}


void GCoderTestCase::testGridPath()
{
	BOOST_LOG_TRIVIAL(trace)<< "Starting:" <<__FUNCTION__ << endl;

	Configuration config;
	config["FileWriterOperation"]["filename"] = SINGLE_EXTRUDER_GRID_PATH;
	config["FileWriterOperation"]["format"]= ".gcode";

	// load 1 extruder
	configureSingleExtruder(config);

	PathData *path = new PathData(0.15);

	srand( time(NULL) );
	int lineCount = 20;
	double lowerX = -30 + 10.0 * ((double) rand()) / RAND_MAX;
	double lowerY = -30 + 10.0 * ((double) rand()) / RAND_MAX;

	double dx = 20.0;
	double dy = 20.0;

	initHorizontalGridPath(*path, lowerX, lowerY, dx, dy, 20);

	vector<PathData*> datas;
	datas.push_back((PathData*)path);
	run_tool_chain(config, datas);

	// cleanup the data
	for(std::vector<PathData*>::iterator it = datas.begin(); it != datas.end(); it++)
	{
		PathData* data = *it;
		//data->release();
	}

	CPPUNIT_ASSERT( ifstream(SINGLE_EXTRUDER_WITH_PATH) );
	BOOST_LOG_TRIVIAL(trace)<< "Exiting:" <<__FUNCTION__ << endl;
}

int random(int start, int range)
{
	int r = rand();
	r = r / (RAND_MAX / range );
	return start + r;
}

void GCoderTestCase::testMultiGrid()
{
	BOOST_LOG_TRIVIAL(trace)<< "Starting:" <<__FUNCTION__ << endl;

	Configuration config;
	config["FileWriterOperation"]["filename"] = SINGLE_EXTRUDER_MULTI_GRID_PATH;
	config["FileWriterOperation"]["format"]= ".gcode";

	// load 1 extruder
	configureSingleExtruder(config);

	vector<PathData*> datas;
	srand( time(NULL) );

	int lineCount = 20;
	double lowerX = -35 + random(-10, 20);
	double lowerY = -35 + random(-10, 20); // 10.0 * ((double) rand()) / RAND_MAX;
	double firstLayerH = 0.11;
	double layerH = 0.35;
	bool horizontal = true;
	double dx = 20.0;
	double dy = 20.0;

	for(int currentLayer=0; currentLayer < 200; currentLayer++)
	{
		PathData *path = new PathData(currentLayer * layerH + firstLayerH);
		if(horizontal)
			initHorizontalGridPath(*path, lowerX, lowerY, dx, dy, 20);
		else
			initVerticalGridPath(*path, lowerX, lowerY, dx, dy, 20);
		datas.push_back((PathData*)path);
		horizontal = !horizontal;
	}
	run_tool_chain(config, datas);

	// cleanup the data
	for(std::vector<PathData*>::iterator it = datas.begin(); it != datas.end(); it++)
	{
		PathData* data = *it;
		//data->release();
	}

	CPPUNIT_ASSERT( ifstream(SINGLE_EXTRUDER_WITH_PATH) );
	BOOST_LOG_TRIVIAL(trace)<< "Exiting:" <<__FUNCTION__ << endl;
}

PathData * createPathFromTubes(const std::vector<Segment> &tubes, Scalar z)
{
	// paths R us
	PathData *pathData;
	pathData = new PathData(z);

	pathData->extruderSlices.push_back(ExtruderSlice());


	Polygons& paths = pathData->extruderSlices[0].infills;
	size_t tubeCount = tubes.size();
	for (int i=0; i< tubeCount; i++)
	{
		const Segment &segment = tubes[i];

		cout << "SEGMENT " << i << "/" << tubeCount << endl;
		paths.push_back(Polygon());
		Polygon &poly = paths[paths.size()-1];

		Vector2 p0 (segment.a.x, segment.a.y);
		Vector2 p1 (segment.b.x, segment.b.y);

		poly.push_back(p0);
		poly.push_back(p1);

	}
	return pathData;
}


void GCoderTestCase::testKnot()
{
	cout << endl;

	string modelFile = "inputs/3D_Knot.stl";
	std::string outDir = "test_cases/GCoderTestCase/output";


	MyComputer myComputer;
	cout << endl;
	cout << endl;
	cout << "behold!" << endl;
	cout << modelFile << "\" has begun at " << myComputer.clock.now() << endl;

	std::string stlFiles = myComputer.fileSystem.removeExtension(myComputer.fileSystem.ExtractFilename(modelFile));
	stlFiles += "_";

	std::string scadFile = outDir;
	scadFile += myComputer.fileSystem.getPathSeparatorCharacter();
	scadFile += myComputer.fileSystem.ChangeExtension(myComputer.fileSystem.ExtractFilename(modelFile), ".scad" );

	std::string stlPrefix = outDir;
	stlPrefix += myComputer.fileSystem.getPathSeparatorCharacter();
	stlPrefix += stlFiles.c_str();
	cout << endl << endl;
	cout << modelFile << " to " << stlPrefix << "*.stl and " << scadFile << endl;

	Configuration config;
	config["FileWriterOperation"]["filename"] = SINGLE_EXTRUDER_KNOT;
	config["FileWriterOperation"]["format"]= ".gcode";

	// load 1 extruder
	configureSingleExtruder(config);
	configureSlicer(config);

	Meshy mesh(config["slicer"]["firstLayerZ"].asDouble(), config["slicer"]["layerH"].asDouble()); // 0.35
	loadMeshyFromStl(mesh, modelFile.c_str());

	std::vector< TubesInSlice > allTubes;
	sliceAndPath(mesh,
			config["slicer"]["layerW"].asDouble(),
			config["slicer"]["tubeSpacing"].asDouble(),
			config["slicer"]["angle"].asDouble(),
			scadFile.c_str(),
			allTubes);

	vector<PathData*> paths;
	for (int i=0; i< allTubes.size(); i++)
	{
		// i is the slice index

		TubesInSlice &tubes = allTubes[i];
		Scalar z = tubes.z;

		for(int j=0; j < tubes.outlines.size(); j++)
		{
			// j is the outline loop index

			std::vector<Segment> &loopTubes = tubes.outlines[j];
			PathData *path = createPathFromTubes(loopTubes, z);
			paths.push_back(path);
		}

		PathData *path = createPathFromTubes(tubes.infill, z);
		paths.push_back(path);
	}

	cout << "Sliced until " << myComputer.clock.now() << endl;
	cout << endl;

	srand( time(NULL) );
	run_tool_chain(config, paths);

	// cleanup the data
	for(std::vector<PathData*>::iterator it = paths.begin(); it != paths.end(); it++)
	{
		PathData* path = *it;
		//path->release();
	}


}
