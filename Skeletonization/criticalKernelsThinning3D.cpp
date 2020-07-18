// Source: https://dgtal-team.github.io/doctools-nightly/criticalKernelsThinning3D.html
// $ cmake -DCMAKE_BUILD_TYPE=Release ..
// $ ./criticalKernelsThinning3D --input /mnt/c/Code/voxels.txt --select dmax --skel 1isthmus --persistence 1 --verbose --exportTXT skeleton.txt

#include <iostream>
#include <chrono>
#include <unordered_map>

#include <DGtal/base/Common.h>
#include <DGtal/helpers/StdDefs.h>
#include <DGtal/io/readers/GenericReader.h>
#include <DGtal/io/writers/GenericWriter.h>
#include "DGtal/images/imagesSetsUtils/SetFromImage.h"
#include "DGtal/images/SimpleThresholdForegroundPredicate.h"
#include "DGtal/images/ImageSelector.h"
#include "DGtal/images/imagesSetsUtils/ImageFromSet.h"

#include <DGtal/topology/SurfelAdjacency.h>
#include <DGtal/io/boards/Board2D.h>
#include <DGtal/topology/CubicalComplex.h>
#include <DGtal/topology/CubicalComplexFunctions.h>

#include <DGtal/topology/VoxelComplex.h>
#include <DGtal/topology/VoxelComplexFunctions.h>
#include "DGtal/topology/NeighborhoodConfigurations.h"
#include "DGtal/topology/tables/NeighborhoodTables.h"

  // boost::program_options
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
// Distance Map
#include "DGtal/geometry/volumes/distance/ExactPredicateLpSeparableMetric.h"
#include "DGtal/geometry/volumes/distance/VoronoiMap.h"
#include "DGtal/geometry/volumes/distance/DistanceTransformation.h"
using namespace DGtal;
using namespace std;
using namespace DGtal::Z3i;
namespace po = boost::program_options;

int main(int argc, char* const argv[]) {

    /*-------------- Parse command line -----------------------------*/
    po::options_description general_opt("Allowed options are: ");
    general_opt.add_options()
        ("help,h", "Display this message.")
        ("input,i", po::value<string>()->required(), "Input vol file.")
        ("skel,s", po::value<string>()->default_value("1isthmus"), "Type of skeletonization. Options: 1isthmus, isthmus, end, ulti.")
        ("select,c", po::value<string>()->default_value("dmax"), "Select the ordering for skeletonization. Options: dmax, random, first")
        ("foreground,f", po::value<string>()->default_value("black"), "Foreground color in binary image")
        ("thresholdMin,m", po::value<int>()->default_value(0), "Threshold min (excluded) to define binary shape")
        ("thresholdMax,M", po::value<int>()->default_value(255), "Threshold max (included) to define binary shape")
        ("persistence,p", po::value<int>()->default_value(0), "Persistence value, implies use of persistence algorithm if p>=1")
        ("profile", po::bool_switch()->default_value(false), "Profile algorithm")
        ("verbose,v", po::bool_switch()->default_value(false), "Verbose output")
        ("exportTXT,e", po::value<std::string>(), "Export the resulting set of points in a simple (sequence of discrete point (sdp)).")
        ("visualize,t", po::bool_switch()->default_value(false), "Visualize result in viewer");
    bool parseOK = true;
    po::variables_map vm;

    try {
        po::store(po::parse_command_line(argc, argv, general_opt), vm);
        po::notify(vm);
    }
    catch (const exception& ex) {
        parseOK = false;
        trace.info() << "Error checking program options: " << ex.what() << std::endl;
    }

    if (!parseOK || vm.count("help") || !vm.count("input"))
    {
        trace.info() <<
            "Compute the thinning of a volume using the CriticalKernels framework" << std::endl
            << std::endl << "Basic usage: " << std::endl
            << "criticalKernelsThinning3D --input <volFileName> --skel <ulti,end, 1isthmus, isthmus> --select "
            " [ -f <white,black> -m <minlevel> -M <maxlevel> -v ] "
            " [--persistence <value> ]" << std::endl
            << "options for --skel {ulti end 1isthmus isthmus}" << std::endl
            << "options for --select = {dmax random first}" << std::endl
            << general_opt << "\n"
            << " Example: \n"
            << "criticalKernelsThinning3D --input ${DGtal}/examples/samples/Al.100.vol --select dmax --skel 1isthmus --persistence 1 --visualize --verbose --outputImage ./Al100_dmax_1isthmus_p1.vol \n";
        return 0;
    }
    //Parse options
    string filename = vm["input"].as<string>();
    bool verbose = vm["verbose"].as<bool>();
    bool visualize = vm["visualize"].as<bool>();
    bool profile = vm["profile"].as<bool>();
    int thresholdMin = vm["thresholdMin"].as<int>();
    int thresholdMax = vm["thresholdMax"].as<int>();
    int persistence = vm["persistence"].as<int>();
    if (vm.count("persistence") && persistence < 0)
        throw po::validation_error(po::validation_error::invalid_option_value, "persistence");
    string foreground = vm["foreground"].as<string>();
    if (vm.count("foreground") && (!(foreground == "white" || foreground == "black")))
        throw po::validation_error(po::validation_error::invalid_option_value, "foreground");

    string sk_string = vm["skel"].as<string>();
    if (vm.count("skel") &&
        (!(sk_string == "ulti" || sk_string == "end" ||
            sk_string == "isthmus" || sk_string == "1isthmus"))
        )
        throw po::validation_error(po::validation_error::invalid_option_value, "skel");
    string select_string = vm["select"].as<string>();
    if (vm.count("select") &&
        (!(select_string == "random" || select_string == "dmax" ||
            select_string == "first"))
        )
        throw po::validation_error(po::validation_error::invalid_option_value, "select");
    /*-------------- End of parse -----------------------------*/

    if (verbose) {
        std::cout << "Skel: " << sk_string << std::endl;
        std::cout << "Select: " << select_string << std::endl;
        std::cout << "Persistence: " << persistence << std::endl;
        std::cout << "Input: " << filename << std::endl;
    }
    trace.beginBlock("Reading input");
    using Domain = Z3i::Domain;
    using Image = ImageSelector<Z3i::Domain, unsigned char>::Type;
    const Domain::Space::Point A(0, 0, 0);
    const Domain::Space::Point B(512, 512, 512);

    Domain domain(A, B);
    DigitalSet image_set(domain);

    std::ifstream file(filename, std::ifstream::in);
    if (!file.is_open())
    {
        return 1;
    }
    int number_voxels;
    file >> number_voxels;

    for (int i = 0; i < number_voxels; i++)
    {
        int x, y, z;
        file >> x >> y >> z;

        image_set.insert(Domain::Space::Point(x, y, z));
    }

    file.close();
    trace.endBlock();

    // Create a VoxelComplex from the set
    using DigitalTopology = DT26_6;
    using DigitalSet =
        DGtal::DigitalSetByAssociativeContainer<Domain,
        std::unordered_set< typename Domain::Point> >;
    using Complex = DGtal::VoxelComplex<KSpace>;

    auto& sk = sk_string;
    KSpace ks;
    KSpace::Point d1(KSpace::Point::diagonal(1));
    ks.init(domain.lowerBound() - d1,
        domain.upperBound() + d1, true);

    trace.beginBlock("construct with table");
    Complex vc(ks);
    vc.construct(image_set, functions::loadTable(simplicity::tableSimple26_6));
    trace.endBlock();
    trace.beginBlock("load isthmus table");
    boost::dynamic_bitset<> isthmus_table;
    if (sk == "isthmus")
        isthmus_table = *functions::loadTable(isthmusicity::tableIsthmus);
    else if (sk == "1isthmus")
        isthmus_table = *functions::loadTable(isthmusicity::tableOneIsthmus);
    auto pointMap = *functions::mapZeroPointNeighborhoodToConfigurationMask<Point>();

    trace.endBlock();
    using namespace DGtal::functions;
    // SKEL FUNCTION:
    std::function< bool(const Complex&, const Cell&) > Skel;
    if (sk == "ulti") Skel = skelUltimate<Complex>;
    else if (sk == "end") Skel = skelEnd<Complex>;
    else if (sk == "isthmus" || sk == "1isthmus")
        Skel = [&isthmus_table, &pointMap](const Complex& fc,
            const Complex::Cell& c) {
                return skelWithTable(isthmus_table, pointMap, fc, c);
    };
    else throw std::runtime_error("Invalid skel string");
    auto start = std::chrono::system_clock::now();

    // SELECT FUNCTION
    /*
     * Calculate distance map even if not requested:
     */
    trace.beginBlock("Create Distance Map");
    using L3Metric = ExactPredicateLpSeparableMetric<Z3i::Space, 3>;
    using DT = DistanceTransformation<Z3i::Space, DigitalSet, L3Metric>;
    L3Metric l3;
    DT dt(domain, image_set, l3);
    trace.endBlock();

    std::function< std::pair<typename Complex::Cell, typename Complex::Data>(const Complex::Clique&) > Select;
    auto& sel = select_string;
    if (sel == "random") Select = selectRandom<Complex>;
    else if (sel == "first") Select = selectFirst<Complex>;
    else if (sel == "dmax") {
        Select =
            [&dt](const Complex::Clique& clique) {
            return selectMaxValue<DT, Complex>(dt, clique);
        };
    }
    else throw std::runtime_error("Invalid skel string");

    trace.beginBlock("Thinning");
    Complex vc_new(ks);
    if (persistence == 0)
        vc_new = asymetricThinningScheme< Complex >(
            vc, Select, Skel, verbose);
    else
        vc_new = persistenceAsymetricThinningScheme< Complex >(
            vc, Select, Skel, persistence, verbose);
    trace.endBlock();

    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds> (end - start);
    if (profile) std::cout << "Time elapsed: " << elapsed.count() << std::endl;


    DigitalSet thin_set(domain);
    vc_new.dumpVoxels(thin_set);
    const auto& all_set = image_set;

    if (vm.count("exportTXT"))
    {
        std::ofstream out;
        out.open(vm["exportTXT"].as<std::string>().c_str());
        out << thin_set.size() << std::endl;
        for (auto& p : thin_set)
        {
            out << p[0] << " " << p[1] << " " << p[2] << std::endl;
        }
        out.close();
    }

    return 0;
}
