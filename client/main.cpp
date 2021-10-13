#include <Windows.h>
#include <iostream>
#include <random>
#include <string>
#include <sstream>
#include <thread>
#include <limits>
#include <filesystem>
#include <unordered_map>
#include <sddl.h>
#include <WinInet.h>

#pragma comment(lib, "wininet")

#include "json.hpp"
#include "xorstr.h"

#include "utilities.hpp"
#include "modules.hpp"
#include "server.hpp"

auto main() -> int
{
    utilities::setup_console();
    
    if (!server::login_user())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::exit(0);
    }

    // this is not nice but idc

    if (utilities::get_choice(xorstr("server | would you like to update your config?->")))
    {
        clicker::key = utilities::get_key(xorstr("\nclicker | toggle key->"));
        clicker::clicks_per_second = utilities::get_float(xorstr("clicker | clicks per second->"));
        jitter::range = utilities::get_float(xorstr("clicker | jitter range->"));

        doubler::key = utilities::get_key(xorstr("\ndouble | toggle key->"));
        doubler::min_clicks_per_second = utilities::get_float(xorstr("double | minimum clicks per second->"));
        doubler::max_clicks_per_second = utilities::get_float(xorstr("double | maximum clicks per second->"));
        doubler::chance = utilities::get_float(xorstr("double | chance->"));

        config::add_item(xorstr("clicker"), xorstr("key"), clicker::key);
        config::add_item(xorstr("clicker"), xorstr("clicks_per_second"), clicker::clicks_per_second);
        config::add_item(xorstr("clicker"), xorstr("jitter_range"), jitter::range);

        config::add_item(xorstr("double"), xorstr("key"), doubler::key);
        config::add_item(xorstr("double"), xorstr("min_clicks_per_second"), doubler::min_clicks_per_second);
        config::add_item(xorstr("double"), xorstr("max_clicks_per_second"), doubler::max_clicks_per_second);
        config::add_item(xorstr("double"), xorstr("chance"), doubler::chance);

        server::save_data();
    }
    else
    {
        server::load_data();

        std::printf(xorstr("\nclicker | toggle key->%x\n"), config::get_item<int>(xorstr("clicker"), xorstr("key")));
        std::printf(xorstr("clicker | clicks per second->%.2f\n"), config::get_item<float>(xorstr("clicker"), xorstr("clicks_per_second")));
        std::printf(xorstr("clicker | jitter->%.2f\n"), config::get_item<float>(xorstr("clicker"), xorstr("jitter_range")));

        std::printf(xorstr("\ndouble | toggle key->%x\n"), config::get_item<int>(xorstr("double"), xorstr("key")));
        std::printf(xorstr("double | minimum clicks per second->%.2f\n"), config::get_item<float>(xorstr("double"), xorstr("min_clicks_per_second")));
        std::printf(xorstr("double | maximum clicks per second->%.2f\n"), config::get_item<float>(xorstr("double"), xorstr("max_clicks_per_second")));
        std::printf(xorstr("double | chance->%.2f\n"), config::get_item<float>(xorstr("double"), xorstr("chance")));

        clicker::key = config::get_item<int>(xorstr("clicker"), xorstr("key"));
        clicker::clicks_per_second = config::get_item<float>(xorstr("clicker"), xorstr("clicks_per_second"));
        jitter::range = config::get_item<float>(xorstr("clicker"), xorstr("jitter_range"));

        doubler::key = config::get_item<int>(xorstr("double"), xorstr("key"));
        doubler::min_clicks_per_second = config::get_item<float>(xorstr("double"), xorstr("min_clicks_per_second"));
        doubler::max_clicks_per_second = config::get_item<float>(xorstr("double"), xorstr("max_clicks_per_second"));
        doubler::chance = config::get_item<float>(xorstr("double"), xorstr("chance"));
    }

    const auto setup_modules = [&]() -> int
    {
        std::printf(xorstr("\nmodules | starting threads\n"));

        keybinds::add(&clicker::key, &clicker::enabled);
        keybinds::add(&doubler::key, &doubler::enabled);

        clicker::run_thread();
        jitter::run_thread();
        doubler::run_thread();
        keybinds::run_thread();

        std::printf(xorstr("modules | successfully started threads\n"));
        std::printf(xorstr("modules | press shift + insert to hide window\n"));
        std::printf(xorstr("modules | press shift + delete to destruct\n"));

        while (true)
        {
            if (utilities::is_pressed(VK_SHIFT))
            {
                if (utilities::is_pressed(VK_DELETE))
                {
                    destruct::run_thread();
                }

                // dont spam the window lol

                static auto status = false;

                if (utilities::is_pressed(VK_INSERT) && !status)
                {
                    utilities::toggle_console();
                    status = true;
                }

                if (!utilities::is_pressed(VK_INSERT) && status)
                {
                    status = false;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    };

    return setup_modules();
}
