namespace utilities
{
    std::mt19937 mersenne{ (std::mt19937::result_type)std::time(nullptr) };

    auto random_int(int min, int max) -> int
    {
        std::uniform_int_distribution gen{ min, max };
        return gen(mersenne);
    }

    auto random_float(float min, float max) -> float
    {
        std::uniform_real_distribution gen{ min, max };
        return gen(mersenne);
    }

    auto is_pressed(int key) -> bool
    {
        return GetAsyncKeyState(key) & 0x8000;
    }

    auto send_click_down(POINT pos) -> bool
    {
        return PostMessageA(GetForegroundWindow(), WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pos.x, pos.y));
    }

    auto send_click_up(POINT pos) -> bool
    {
        return PostMessageA(GetForegroundWindow(), WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos.x, pos.y));
    }

    auto handle_window_class(const char* klass, POINT& screen_to_client) -> bool
    {
        auto window = FindWindowA(klass, NULL);
        return GetForegroundWindow() == window && ScreenToClient(window, &screen_to_client);
    }

    auto message_box(const char* text) -> int
    {
        return MessageBoxA(0, text, 0, 0);
    }

    auto setup_console() -> void
    {
        SetConsoleTitleA("");
    }

    auto set_cursor_visibility(bool visible) -> void
    {
        auto cursor = CONSOLE_CURSOR_INFO();
        auto console = GetStdHandle(STD_OUTPUT_HANDLE);

        cursor.bVisible = visible;
        cursor.dwSize = visible ? 10 : 20;

        SetConsoleCursorInfo(console, &cursor);
    }

    auto flush_buffer() -> void
    {
        FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    }

    auto set_console_visibility(bool shown) -> void
    {
        ShowWindow(GetConsoleWindow(), shown ? SW_SHOW : SW_HIDE);
    }

    auto toggle_console() -> void
    {
        ShowWindow(GetConsoleWindow(), IsWindowVisible(GetConsoleWindow()) != FALSE ? SW_HIDE : SW_SHOW);
    }

    auto get_choice(const char* text) -> bool
    {
        auto input = std::string();

        std::printf(text);

        while (std::getline(std::cin, input))
        {
            if (input == xorstr("y") || input == xorstr("Y"))
            {
                return true;
            }

            if (input == xorstr("n") || input == xorstr("N"))
            {
                return false;
            }

            std::printf(text);
        }
    }

    auto get_string(const char* text) -> std::string
    {
        auto input = std::string();

        std::printf(text);

        while (std::getline(std::cin, input))
        {
            if (!input.empty())
            {
                return input;
            }

            std::printf(text);
        }
    }

    auto get_float(const char* text) -> float
    {
        auto input = std::string();
        auto ret = float();

        std::printf(text);

        while (std::getline(std::cin, input))
        {
            std::stringstream ss(input);

            if (ss >> ret)
            {
                if (ss.eof())
                {
                    break;
                }
            }

            std::printf(text);
        }

        return ret;
    }

    auto get_key(const char* text) -> int
    {
        std::printf(text);
        auto key = int(0);

        //dont get last press
        for (auto i = 3; i < 256; i++)
        {
            keybd_event(i, 0, KEYEVENTF_KEYUP, 0);
        }

        while (key == 0)
        {
            for (auto i = 3; i < 256; i++)
            {
                if (utilities::is_pressed(i & SHRT_MAX) && key == 0)
                {
                    key = i;
                    std::printf(xorstr("%x\n"), key);
                }
            }
        }

        utilities::flush_buffer();

        return key;
    }

    auto base64_decode(std::string encoded_string) -> std::string
    {
        static const auto base64_chars = std::string 
        {
            xorstr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/")
        };

        const auto is_base64 = [&](unsigned char c) -> bool
        {
            return (isalnum(c) || (c == '+') || (c == '/'));
        };

        auto in_len = encoded_string.size();

        auto i = 0;
        auto j = 0;
        auto in_ = 0;

        unsigned char char_array_4[4], char_array_3[3];
        auto ret = std::string();

        while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) 
        {
            char_array_4[i++] = encoded_string[in_]; in_++;

            if (i == 4) 
            {
                for (i = 0; i < 4; i++)
                {
                    char_array_4[i] = base64_chars.find(char_array_4[i]);
                }

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                {
                    ret += char_array_3[i];
                }

                i = 0;
            }
        }

        if (i) 
        {
            for (j = i; j < 4; j++)
            {
                char_array_4[j] = 0;
            }

            for (j = 0; j < 4; j++)
            {
                char_array_4[j] = base64_chars.find(char_array_4[j]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++)
            {
                ret += char_array_3[j];
            }
        }

        return ret;
    }
}
