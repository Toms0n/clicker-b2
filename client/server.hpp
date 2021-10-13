namespace config
{
    nlohmann::json json_object;

    template <typename t>
    auto add_item(std::string catagory, std::string value, t val) -> void
    {
        json_object[catagory][value] = val;
    }

    template <typename t>
    auto get_item(std::string catagory, std::string value) -> t
    {
        return json_object[catagory][value].get<t>();
    }

    auto dump() -> std::string
    {
        return json_object.dump();
    }

    auto load(std::string data) -> void
    {
        json_object = nlohmann::json::parse(data);
    }
}

namespace server
{
    auto request(std::string param) -> std::string 
	{
        auto base_url = std::string(xorstr("lilyiscool.com"));

		auto h_internet = InternetOpenA(xorstr("Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.107 Safari/537.36"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
		auto h_connect = InternetConnectA(h_internet, base_url.data(), 80, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
		
		const char* accept_types[] = { "text/*", 0 };

		auto h_request = HttpOpenRequestA(h_connect, xorstr("GET"), param.data(), NULL, NULL, accept_types, 0, 0);
        auto request_sent = HttpSendRequestA(h_request, NULL, 0, NULL, 0);

		auto response = std::string();
		auto keep_reading = true;
        auto bytes_read = DWORD(-1);

        const auto buf_size = 1024;
        auto buf = new char[buf_size];

		while (keep_reading && bytes_read != 0)
		{
            keep_reading = InternetReadFile(h_request, buf, buf_size, &bytes_read);
            response.append(buf, bytes_read);
		}

		return response;
    }

    auto get_hardware_id() -> std::string
    {
        auto h_token = HANDLE();
        OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &h_token);

        auto dw_buffer_size = DWORD();
        GetTokenInformation(h_token, TokenUser, nullptr, 0, &dw_buffer_size);

        auto buffer = std::vector<BYTE>(dw_buffer_size);
        const auto p_token_user = (PTOKEN_USER) &buffer[0];

        GetTokenInformation(h_token, TokenUser, p_token_user, dw_buffer_size, &dw_buffer_size);
        IsValidSid(p_token_user->User.Sid);
        CloseHandle(h_token);

        auto psz_sid = new char[255];

        ConvertSidToStringSidA(p_token_user->User.Sid, &psz_sid);

        std::string psz_sid_str(psz_sid);
        return std::string(psz_sid_str);
    }

    auto is_valid_response(std::string text) -> bool 
    {
        nlohmann::json obj;

        try 
        {
            obj = nlohmann::json::parse(text);
        }
        catch (std::exception& e) 
        {
            return false;
        }

        if (obj != NULL && obj.is_object())
            return true;
    }

    auto login_user() -> bool
    {
        auto buf = new char[1024];
        auto hwid = server::get_hardware_id();
       
        std::sprintf(buf, xorstr("/clicker.php?login=%s"), hwid.data());

        auto try_login = server::request(buf);
        auto login_decoded = utilities::base64_decode(try_login);

        if (server::is_valid_response(login_decoded))
        {
            auto login_json = nlohmann::json::parse(login_decoded);

            if (login_json[xorstr("status")] != xorstr("success"))
            {
                if (login_json[xorstr("reason")] == xorstr("not_registered"))
                {
                    auto user = utilities::get_string(xorstr("server | enter username->"));
                    std::sprintf(buf, xorstr("/clicker.php?register=%s&hwid=%s"), user.data(), hwid.data());

                    auto try_register = server::request(buf);
                    auto register_decoded = utilities::base64_decode(try_register);

                    if (server::is_valid_response(register_decoded))
                    {
                        auto register_json = nlohmann::json::parse(register_decoded);

                        if (register_json[xorstr("status")] == xorstr("success"))
                        {
                            std::printf(xorstr("server | registered sucessfully, welcome %s\n"), user.data());
                            return true;
                        }
                        else
                        {
                            std::printf(xorstr("server | internal server error, please try again (0)\n"));
                            return false;
                        }
                    }
                    else
                    {
                        std::printf(xorstr("server | internal server error, please try again (1)\n"));
                        return false;
                    }
                }

                if (login_json[xorstr("reason")] == xorstr("hwid_error"))
                {
                    std::printf(xorstr("server | hwid invalid, request a hwid reset\n"));
                    return false;
                }

                if (login_json[xorstr("reason")] == xorstr("invalid_user"))
                {
                    std::printf(xorstr("server | internal server error, please try again (2)\n"));
                    return false;
                }
            }
            else
            {
                auto username = login_json[xorstr("username")].get<std::string>();
                std::printf(xorstr("server | logged in sucessfully, welcome %s\n"), username.data());
                return true;
            }
        }
        else
        {
            std::printf(xorstr("server | internal server error, please try again (3)\n"));
            return false;
        }

        return false;
    }

    auto load_data() -> int
    {
        std::printf(xorstr("server | downloading config data\n"));

        auto buf = new char[1024];
        auto hwid = server::get_hardware_id();

        std::sprintf(buf, xorstr("/clicker.php?load=%s"), hwid());

        auto config_data = server::request(buf);
        auto config_decoded = utilities::base64_decode(config_data);

        if (server::is_valid_response(config_decoded))
        {
            std::printf(xorstr("server | download successful\n"));
            config::load(config_decoded);
            return 0;
        }
        else
        {
            std::printf(xorstr("server | malformed config data\n"));
            return 1;
        }
    }

    auto save_data() -> int
    {
        std::printf(xorstr("\nserver | uploading config data\n"));

        auto buf = new char[1024];
        auto hwid = server::get_hardware_id();
        auto config_data = config::dump();
        
        std::sprintf(buf, xorstr("/clicker.php?save=%s&data=%s"), hwid.data(), config_data.data());

        auto config_ret = server::request(buf);
        auto config_decoded = utilities::base64_decode(config_ret);

        if (server::is_valid_response(config_decoded))
        {
            auto config_json = nlohmann::json::parse(config_decoded);

            if (config_json[xorstr("status")] == xorstr("success"))
            {
                std::printf(xorstr("server | upload successful\n"));
                return 0;
            }
            else
            {
                std::printf(xorstr("server | upload failed\n"));
                return 1;
            }
        }
        else
        {
            std::printf(xorstr("server | internal server error, try again later (4)\n"));
            return 1;
        }
    }
}