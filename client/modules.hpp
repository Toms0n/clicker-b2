namespace clicker
{
    auto enabled = false;
    auto key = 0;
    auto clicks_per_second = 12.f;

    // credits jesse, pengu, thx

    namespace random
    {
        static auto edited_cps = 0, reset_value = 0, clicks = 0, return_ms = 0;

        auto get_time() -> int
        {
            if (random::clicks >= random::reset_value || random::clicks == 0)
            {
                random::reset_value = utilities::random_int(1, 5);
                random::edited_cps = clicker::clicks_per_second + utilities::random_int(-3, 3);
                random::clicks = 0;
            }

            random::clicks++;
            random::return_ms = utilities::random_int(225, 275);

            return random::return_ms / random::edited_cps * 2;
        }
    }

    auto run_thread() -> void
    {
        std::thread instance([]
            {
                while (true)
                {
                    if (clicker::enabled && utilities::is_pressed(VK_LBUTTON))
                    {
                        if (utilities::random_int(0, 100) <= 2)
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(clicker::random::get_time() * 3));
                        }

                        static auto cursor_pos = POINT();

                        if (utilities::handle_window_class(xorstr("LWJGL"), cursor_pos))
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(clicker::random::get_time()));
                            utilities::send_click_down(cursor_pos);

                            std::this_thread::sleep_for(std::chrono::milliseconds(clicker::random::get_time()));
                            utilities::send_click_up(cursor_pos);
                        }
                    }
                    else
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                }
            });

        return instance.detach();
    }
}

namespace jitter
{
    auto range = 0.f;

    auto run_thread() -> void
    {
        std::thread instance([]
            {
                while (true)
                {
                    static auto cursor_pos = POINT();

                    if (clicker::enabled && jitter::range > 0 && utilities::handle_window_class(xorstr("LWJGL"), cursor_pos) && utilities::is_pressed(VK_LBUTTON))
                    {
                        mouse_event(MOUSEEVENTF_MOVE, utilities::random_int(-jitter::range, jitter::range), utilities::random_int(-jitter::range, jitter::range), 0, 0);
                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    }
                    else
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                }
            });

        return instance.detach();
    }
}

namespace keybinds
{
    struct bind
    {
        int* key;
        bool* value;
        bool* status;
    };

    std::vector<bind> binds;

    auto add(int* key, bool* value) -> void
    {
        auto bind_struct = keybinds::bind();

        bind_struct.key = key;
        bind_struct.status = new bool(false);
        bind_struct.value = value;

        binds.emplace_back(bind_struct);
    }

    auto run_thread() -> void
    {
        std::thread instance([]
            {
                while (true)
                {
                    for (auto object : binds)
                    {
                        if (utilities::is_pressed(*object.key) && !(*object.status))
                        {
                            *object.status = true;
                        }
                        else if (!utilities::is_pressed(*object.key) && *object.status)
                        {
                            *object.value = !(*object.value);
                            *object.status = false;
                        }
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            });

        return instance.detach();
    }
}

namespace doubler
{
    auto enabled = false;
    auto key = 0;

    auto min_clicks_per_second = 0.f;
    auto max_clicks_per_second = 0.f;
    auto chance = 0.f;

    // whatever

    namespace random
    {
        auto get_time() -> int
        {
            return utilities::random_int(35, 75) / 2;
        }
    }

    auto run_thread() -> void
    {
        std::thread instance([]
            {
                while (true)
                {
                    static auto status = false;
                    static auto current_cps = 0;

                    static auto cursor_pos = POINT();
                    static auto time = std::size_t(0);

                    if (doubler::enabled && utilities::handle_window_class(xorstr("LWJGL"), cursor_pos))
                    {
                        if (GetTickCount64() - time > 1000)
                        {
                            time = GetTickCount64();
                            current_cps = 0;
                        }

                        if (utilities::is_pressed(VK_LBUTTON) && !status)
                        {
                            current_cps++;

                            status = true;

                            if (current_cps > doubler::min_clicks_per_second && current_cps < doubler::max_clicks_per_second)
                            {
                                if (utilities::random_int(0, 100) <= doubler::chance)
                                {
                                    auto range = abs(doubler::max_clicks_per_second - doubler::min_clicks_per_second);

                                    std::this_thread::sleep_for(std::chrono::milliseconds(current_cps <= (range / 2) ? utilities::random_int(5, 10) : doubler::get_time()));
                                    utilities::send_click_down(cursor_pos);

                                    std::this_thread::sleep_for(std::chrono::milliseconds(current_cps <= (range / 2) ? utilities::random_int(5, 10) : doubler::get_time()));
                                    utilities::send_click_up(cursor_pos);
                                }
                            }
                        }
                        else if (!utilities::is_pressed(VK_LBUTTON) && status)
                        {
                            status = false;
                        }

                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                }
            });

        return instance.detach();
    }
}

namespace destruct
{
    auto get_path() -> std::string
    {
        auto path = new char[MAX_PATH];
        GetModuleFileNameA(0, path, MAX_PATH);
        return path;
    }

    auto get_name() -> std::string
    {
        auto path = destruct::get_path();
        return path.substr(path.rfind(xorstr("\\")) + 1);
    }

    auto clear_prefetch() -> void
    {   
        auto dir_buf = new char[255];

        if (!GetWindowsDirectoryA(dir_buf, 255))
            return;

        auto dir = std::string(dir_buf) + xorstr("\\Prefetch\\");
        auto name = destruct::get_name();

        std::transform(name.begin(), name.end(), name.begin(), toupper);

        for (const auto entry : std::filesystem::directory_iterator(dir))
        {
            auto w_path = std::wstring(entry.path());
            auto path = std::string(w_path.begin(), w_path.end());

            if (path.find(name) != std::string::npos)
            {
                std::remove(path.data());
            }

            if (path.find(xorstr("WMIC")) != std::string::npos)
            {
                std::remove(path.data());
            }
        }
    }

    auto delete_file() -> void
    {
        auto si = STARTUPINFO{ 0 };
        auto pi = PROCESS_INFORMATION{ 0 };
        auto cmd = new char[1024];

        std::sprintf(cmd, xorstr("cmd.exe /C ping 1.1.1.1 -n 1 -w 3000 > Nul & Del /f /q \"%s\""), destruct::get_path().data());

        CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }

    // this is basic but idc make a good destruct yourself

    auto run_thread() -> void
    {
        destruct::clear_prefetch();
        destruct::delete_file();

        std::exit(0);
    }
}