#include "config.hpp"
#include <cstdio>
#include <vector>
#include "information_elements.hpp"

class ArgParser
{
public:
    ArgParser(int argc, char **argv);

    bool
    next();

    std::string
    arg();

private:
    int m_argc;
    char **m_argv;
    int m_arg_index = 0;
};

ArgParser::ArgParser(int argc, char **argv)
    : m_argc(argc), m_argv(argv)
{
}

bool
ArgParser::next()
{
    if (m_arg_index == m_argc - 1) {
        return false;
    }
    m_arg_index++;
    return true;
}

std::string
ArgParser::arg()
{
    return m_argv[m_arg_index];
}

static void
usage()
{
    const char *text =
    "Usage: fdsdump [options]\n"
    "  -h         Show this help\n"
    "  -r path    FDS input file\n"
    "  -f expr    Input filter\n"
    "  -of expr   Output filter\n"
    "  -c num     Max number of records to read\n"
    "  -a keys    Aggregator keys (e.g. srcip,dstip,srcport,dstport)\n"
    "  -av values Aggregator values // TODO\n"
    "  -s field   Field to sort on (e.g. bytes, packets, flows)\n"
    "  -n num     Maximum number of records to write\n"
    ;
    fprintf(stderr, text);
}

static void
missing_arg(const char *opt)
{
    fprintf(stderr, "Missing argument for %s", opt);
}

static std::vector<std::string>
string_split(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> pieces;
    std::size_t pos = 0;
    for (;;) {
        std::size_t next_pos = str.find(delimiter, pos);
        if (next_pos == std::string::npos) {
            pieces.emplace_back(str.begin() + pos, str.end());
            break;
        }
        pieces.emplace_back(str.begin() + pos, str.begin() + next_pos);
        pos = next_pos + 1;
    }
    return pieces;
}

static bool
is_one_of(const std::string &value, const std::vector<std::string> values)
{
    for (const auto &v : values) {
        if (v == value) {
            return true;
        }
    }
    return false;
}

static int 
parse_aggregate_key_config(const std::string &options, ViewDefinition &view_def)
{
    view_def = {};

    for (const auto &key : string_split(options, ",")) {
        if (key == "srcip") {
            view_def.key_src_ip = true;
        } else if (key == "dstip") {
            view_def.key_dst_ip = true;
        } else if (key == "srcport") {
            view_def.key_src_port = true;
        } else if (key == "dstport") {
            view_def.key_dst_port = true;
        } else if (key == "proto") {
            view_def.key_protocol = true;
        } else {
            fprintf(stderr, "Invalid aggregation key \"%s\"\n", key.c_str());
            return 1;
        }
    }
    return 0;
}

static int
parse_aggregate_value_config(const std::string &options, ViewDefinition &view_def)
{
    auto values = string_split(options, ",");
    ViewField field;

    for (const auto &value : values) {
        if (value == "packets") {
            field.data_type = DataType::Unsigned64;
            field.pen = IPFIX::iana;
            field.id = IPFIX::packetDeltaCount;
            field.kind = ViewFieldKind::Sum;
            field.name = "packets";
            field.size = sizeof(ViewValue::u64);
            view_def.values_size += sizeof(ViewValue::u64);
        } else if (value == "bytes") {
            field.data_type = DataType::Unsigned64;
            field.pen = IPFIX::iana;
            field.id = IPFIX::octetDeltaCount;
            field.kind = ViewFieldKind::Sum;
            field.name = "bytes";
            field.size = sizeof(ViewValue::u64);
            view_def.values_size += sizeof(ViewValue::u64);
        } else if (value == "flows") {
            field.data_type = DataType::Unsigned64;
            field.kind = ViewFieldKind::FlowCount;
            field.name = "flows";
            field.size = sizeof(ViewValue::u64);
            view_def.values_size += sizeof(ViewValue::u64);
        } else {
            fprintf(stderr, "Invalid aggregation value \"%s\"\n", value.c_str());
            return 1;
        }
        view_def.value_fields.push_back(field);
    }

    return 0;
}

int
config_from_args(int argc, char **argv, Config &config)
{
    config = {};
    config.view_def.key_src_ip = true;
    config.view_def.key_dst_ip = true;
    config.view_def.key_src_port = true;
    config.view_def.key_dst_port = true;
    config.view_def.key_protocol = true;

    ArgParser parser{argc, argv};
    while (parser.next()) {
        if (parser.arg() == "-h") {
            usage();
            return 1;
        } else if (parser.arg() == "-r") {
            if (!parser.next()) {
                missing_arg("-r");
                return 1;
            }
            config.input_file = parser.arg();
        } else if (parser.arg() == "-f") {
            if (!parser.next()) {
                missing_arg("-f");
                return 1;
            }
            config.input_filter = parser.arg();
        } else if (parser.arg() == "-of") {
            if (!parser.next()) {
                missing_arg("-of");
                return 1;
            }
            config.output_filter = parser.arg();
        } else if (parser.arg() == "-c") {
            if (!parser.next()) {
                missing_arg("-c");
                return 1;
            }
            config.max_input_records = std::stoul(parser.arg());
        } else if (parser.arg() == "-a") {
            if (!parser.next()) {
                missing_arg("-a");
                return 1;
            }
            if (parse_aggregate_key_config(parser.arg(), config.view_def) == 1) {
                return 1;
            }
        } else if (parser.arg() == "-av") {
            if (!parser.next()) {
                missing_arg("-av");
                return 1;
            }
            if (parse_aggregate_value_config(parser.arg(), config.view_def) == 1) {
                return 1;
            }
        } else if (parser.arg() == "-n") {
            if (!parser.next()) {
                missing_arg("-n");
                return 1;
            }
            config.max_output_records = std::stoul(parser.arg());
        } else if (parser.arg() == "-s") {
            if (!parser.next()) {
                missing_arg("-s");
                return 1;
            }
            config.sort_field = parser.arg();
            if (!is_one_of(config.sort_field, {"bytes", "packets", "flows"})) {
                fprintf(stderr, "Invalid sort field \"%s\"\n", config.sort_field.c_str());
                return 1;
            }
        } else {
            fprintf(stderr, "Unknown argument %s\n", parser.arg().c_str());
            return 1;
        }
    }

    if (config.input_file.empty()) {
        usage();
        return 1;
    }

    if (config.input_filter.empty()) {
        config.input_filter = "true";
    }

    if (config.output_filter.empty()) {
        config.output_filter = "true";
    }

    return 0;
}