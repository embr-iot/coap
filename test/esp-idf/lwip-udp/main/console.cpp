#include <wifi-console.h>

#include <embr/net/console/args.h>

#include <esp_console.h>

#include <string>

using namespace embr;

using string = const std::string_view;

static console::Args args;

static int coap_console(int argc, char *argv[])
{
    const int nerrors = arg_parse(argc, argv, (void**)&args);

    if(nerrors) return -1;

    string command = args.command->sval[0];

    if(command == "hi") {}

    return -1;
}

esp_err_t coap_console_init()
{
    const esp_console_cmd_t cmd
    {
        .command = "coap",
        .help = "LWIP UDP CoAP control",
        .hint = NULL,
        .func = &coap_console,
        .argtable = &args,
        .func_w_context = nullptr,
        .context = nullptr
    };

    args.command = arg_str1(nullptr, nullptr,
        "<test>",
        "");
    args.arg1 = arg_strn(nullptr, nullptr,
        "<arg1>",
        0, 1,
        "TBD");
    args.arg2 = arg_strn(nullptr, nullptr,
        "<arg2>",
        0, 1,
        "TBD");
    args.end = arg_end(2);
    
    return esp_console_cmd_register(&cmd);
}
