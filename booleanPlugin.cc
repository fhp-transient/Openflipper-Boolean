#include "booleanPlugin.hh"

#include "OpenFlipper/BasePlugin/PluginFunctions.hh"

#include <iostream>
#include <cassert>
#include <string>
#include <fstream>
#include <vector>

// #define QUICKLOAD


BooleanPlugin::BooleanPlugin() : APolygon(nullptr), BPolygon(nullptr)
{
}

void BooleanPlugin::initializePlugin()
{
    QWidget *toolBox = new QWidget();

    QPushButton *loadAPolygonBtn = new QPushButton("&Load A", toolBox);
    QPushButton *loadBPolygonBtn = new QPushButton("&Load B", toolBox);
    QPushButton *Reset = new QPushButton("Reset", toolBox);
    QPushButton *EXECUnion = new QPushButton("&Union", toolBox);
    QPushButton *EXECInter = new QPushButton("&Intersection", toolBox);
    QPushButton *EXECSub = new QPushButton("&Subtraction", toolBox);

    QLabel *label = new QLabel("Iterations:");

    QGridLayout *layout = new QGridLayout(toolBox);
    layout->addWidget(loadAPolygonBtn, 0, 0);
    layout->addWidget(loadBPolygonBtn, 0, 1);
    layout->addWidget(Reset, 0, 2);
    layout->addWidget(EXECUnion, 1, 0);
    layout->addWidget(EXECInter, 1, 1);
    layout->addWidget(EXECSub, 1, 2);
    // layout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding), 2, 0, 1, 2);

    connect(loadAPolygonBtn, &QPushButton::clicked, this, &BooleanPlugin::loadA);
    connect(loadBPolygonBtn, &QPushButton::clicked, this, &BooleanPlugin::loadB);
    connect(Reset, &QPushButton::clicked, this, &BooleanPlugin::reset);
    connect(EXECUnion, &QPushButton::clicked, this, &BooleanPlugin::executeUnion);
    connect(EXECInter, &QPushButton::clicked, this, &BooleanPlugin::executeInter);
    connect(EXECSub, &QPushButton::clicked, this, &BooleanPlugin::executeSub);
    emit addToolbox(tr("Boolean Plugin"), toolBox);
}

void BooleanPlugin::pluginsInitialized() {
    // Variable which will store the id of the newly created object.
    obj_id_1 = -1;
	obj_id_2 = -1;
	result_obj_id = -1;
    PolyLineCollectionObject* obj_1{nullptr}, *obj_2{nullptr}, *result_obj{nullptr};

    // Emit the signal, that we want to create a new object of the specified type plane
    emit addEmptyObject( DATA_POLY_LINE_COLLECTION, obj_id_1 );
    emit addEmptyObject( DATA_POLY_LINE_COLLECTION, obj_id_2 );
    emit addEmptyObject( DATA_POLY_LINE_COLLECTION, result_obj_id );

    // Get the newly created object
    PluginFunctions::getObject(obj_id_1, obj_1);
    PluginFunctions::getObject(obj_id_2, obj_2);
    PluginFunctions::getObject(result_obj_id, result_obj);

    if(obj_1) { log("Create polygon 1!"); } 
    else { log("Fail to create polygon 1!!"); }

    if(obj_2) { log("Create polygon 2!"); } 
    else { log("Fail to create polygon 2!!"); }

    if(result_obj) { log("Create result polygon!"); } 
    else { log("Fail to result polygon!!"); }
}

void BooleanPlugin::loadA()
{
    PolyLineCollectionObject* obj_1{nullptr};
    PluginFunctions::getObject(obj_id_1, obj_1);
    if (obj_1->collection()->n_polylines() > 0)
    {
        log("Object A is not empty");
        return;
    }

    auto filePath = QFileDialog::getOpenFileName(nullptr,
                                                 tr("Choose one file"),
                                                 QString(),
                                                 u8"Plain Text (*.txt *.ppp)");
    if (filePath.isEmpty() || filePath.isNull()) { return; }

    MpA.reset();

    if (!load(filePath.toStdString(), MpA, obj_1->collection())) {
        log("Fail to load file: " + QString(filePath));
        return;
    }
	else
    {
        log("Success to load file: " + QString(filePath));
    }

    emit updatedObject(obj_id_1, UPDATE_ALL);
    log("Load polygon A!");
}

void BooleanPlugin::loadB()
{
    PolyLineCollectionObject* obj_2{nullptr};
    PluginFunctions::getObject(obj_id_2, obj_2);
    if (obj_2->collection()->n_polylines() > 0)
    {
        log("Object B is not empty");
        return;
    }

    auto filePath = QFileDialog::getOpenFileName(nullptr,
                                                 tr("Choose one file"),
                                                 QString(),
                                                 u8"Plain Text (*.txt *.ppp)");
    if (filePath.isEmpty() || filePath.isNull()) { return; }

    MpB.reset();

    if (!load(filePath.toStdString(), MpB, obj_2->collection())) {
        log("Fail to load file: " + QString(filePath));
        return;
    }
	else
    {
        log("Success to load file: " + QString(filePath));
    }

    emit updatedObject(obj_id_2, UPDATE_ALL);
    log("Load polygon B!");
}

void BooleanPlugin::executeUnion()
{
    log("Calculate union...");
    if (obj_id_1 == -1 || obj_id_2 == -1)
    {
        std::cout << "Load first\n";
        return;
    }
    std::vector<std::vector<Point>> ans;
    solve_or(MpA, MpB, ans);
    PolyLine* polygon;
    PolyLineCollectionObject* obj_result{nullptr};
    PluginFunctions::getObject(result_obj_id, obj_result);

    if (!ans.empty())
    {
        for (auto it : ans)
        {
            int id = obj_result->collection()->new_poly_line();
            polygon = obj_result->collection()->polyline(id);
            polygon->clear();
        	for (auto p: it) {
				polygon->add_point(PolyLine::Point(p.x, p.y, 0.0));
        		printf("%lf %lf\n", p.x, p.y);
        	}

        }
    }
    emit updatedObject(result_obj_id, UPDATE_ALL);
    clearSourceObjects();
}

void BooleanPlugin::executeInter()
{
    log("Calculate inter...");
    if (obj_id_1 == -1 || obj_id_2 == -1)
    {
        std::cout << "Load first\n";
        return;
    }
    std::vector<std::vector<Point>> ans;
    solve_and(MpA, MpB, ans);
    PolyLine* polygon;
    PolyLineCollectionObject* obj_result{nullptr};
    PluginFunctions::getObject(result_obj_id, obj_result);

    if (!ans.empty())
    {
        for (auto it : ans)
        {
            int id = obj_result->collection()->new_poly_line();
            polygon = obj_result->collection()->polyline(id);
            polygon->clear();
        	for (auto p: it) {
				polygon->add_point(PolyLine::Point(p.x, p.y, 0.0));
        		printf("%lf %lf\n", p.x, p.y);
        	}

        }
    }
    emit updatedObject(result_obj_id, UPDATE_ALL);
    clearSourceObjects();
}

void BooleanPlugin::executeSub()
{
    log("Calculate sub...");
    if (obj_id_1 == -1 || obj_id_2 == -1)
    {
        std::cout << "Load first\n";
        return;
    }
    std::vector<std::vector<Point>> ans;
    solve_sub(MpA, MpB, ans);
    PolyLine* polygon;
    PolyLineCollectionObject* obj_result{nullptr};
    PluginFunctions::getObject(result_obj_id, obj_result);

    if (!ans.empty())
    {
        for (auto it : ans)
        {
            int id = obj_result->collection()->new_poly_line();
            polygon = obj_result->collection()->polyline(id);
            polygon->clear();
        	for (auto p: it) {
				polygon->add_point(PolyLine::Point(p.x, p.y, 0.0));
        		printf("%lf %lf\n", p.x, p.y);
        	}

        }
    }
    emit updatedObject(result_obj_id, UPDATE_ALL);
    clearSourceObjects();
}

bool BooleanPlugin::load(const std::string &filePath, MPolygon &p, PolyLineCollection* polygons)
{
    auto f = std::fstream(filePath, std::ios::in);
    if (!f.is_open()) { return false; }


    std::string line, flag;
    std::istringstream linestream;

    int cnt = 0;
    std::vector<Point> out, tmp;
    std::vector<std::vector<Point> > in;
    PolyLine* polygon;

    while (getline(f, line))
    {
        if (line[0] == '#') {
            int id = polygons->new_poly_line();
            polygon = polygons->polyline(id);
            polygon->clear();
            if (cnt > 1) {
                in.emplace_back(tmp);
                tmp.clear();
            }
            cnt++;
            continue;
        }

        linestream = std::istringstream(line);
        double x, y;
        const double z = 0.0;
        linestream >> x >> y;

        PolyLine::Point p(x, y, 0.0);
        polygon->add_point(p);
        // printf("x: %lf, y: %lf\n", x, y);
        
        if (cnt == 1) {
            out.emplace_back(x, y);
        } else {
            tmp.emplace_back(x, y);
        }
    }
    if (cnt > 1) in.emplace_back(tmp);
    f.close();

    Edge temp;
    ///out
    for (int i = 0; i < out.size() - 1; i++) {
        temp.LPoint.x = out[i].x;
        temp.LPoint.y = out[i].y;
        temp.RPoint.x = out[i + 1].x;
        temp.RPoint.y = out[i + 1].y;
        p.external_circle.circle_edge.push_back(temp);
    }
    // temp.LPoint.x = out[0].x;
    // temp.LPoint.y = out[0].y;
    // temp.RPoint.x = out[out.size() - 1].x;
    // temp.RPoint.y = out[out.size() - 1].y;
    // p.external_circle.circle_edge.push_back(temp);
    ///in
    for (int i = 0; i < in.size(); i++) {
        Circle tc;
        p.internal_circle.push_back(tc);
        for (int j = 0; j < in[i].size() - 1; j++) {
            temp.LPoint.x = in[i][j].x;
            temp.LPoint.y = in[i][j].y;
            temp.RPoint.x = in[i][j + 1].x;
            temp.RPoint.y = in[i][j + 1].y;
            p.internal_circle[i].circle_edge.push_back(temp);
        }
        // temp.LPoint.x = in[i][0].x;
        // temp.LPoint.y = in[i][0].y;
        // temp.RPoint.x = in[i][in[i].size() - 1].x;
        // temp.RPoint.y = in[i][in[i].size() - 1].y;
        // p.internal_circle[i].circle_edge.push_back(temp);
        // std::reverse(p.internal_circle[i].circle_edge.begin(), p.internal_circle[i].circle_edge.end());
    }

    return true;
}

void BooleanPlugin::clearSourceObjects() {
    PolyLineCollectionObject* obj_1{nullptr}, *obj_2{nullptr};
    PluginFunctions::getObject(obj_id_1, obj_1);
    PluginFunctions::getObject(obj_id_2, obj_2);
    obj_1->collection()->clear();
    obj_2->collection()->clear();
    emit updatedObject(obj_id_1, UPDATE_ALL);
    emit updatedObject(obj_id_2, UPDATE_ALL);
    log("Clear source objects.");
}


void BooleanPlugin::clearResultObject() {
    PolyLineCollectionObject* result_obj{nullptr};
    PluginFunctions::getObject(result_obj_id, result_obj);
    result_obj->collection()->clear();
    emit updatedObject(result_obj_id, UPDATE_ALL);
    log("Clear result object.");
}

void BooleanPlugin::reset()
{
    clearSourceObjects();
    clearResultObject();
}
