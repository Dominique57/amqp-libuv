#include <boost/program_options.hpp>
#include <iostream>
#include <cstdlib>

namespace po = boost::program_options;

po::options_description define_options()
{
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "show usage");
    return desc;
}

po::variables_map parse_options(const po::options_description& desc, int argc,
        char** argv)
{
    po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

    return vm;
}

int run(const po::options_description& desc, const po::variables_map& vm)
{
    if (vm.count("help"))
        std::cout << desc << std::endl;

    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
    try {
        auto const& desc = define_options();
        auto const& vm = parse_options(desc, argc, argv);
        return run(desc, vm);
    } catch (po::error &e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 2;
    }
}

