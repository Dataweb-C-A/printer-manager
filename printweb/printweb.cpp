#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <string>
#include <windows.h>
#include <winspool.h>

#pragma comment(lib, "winspool.lib")

namespace beast = boost::beast;             // from <boost/beast.hpp>
namespace http = beast::http;               // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;     // from <boost/beast/websocket.hpp>
namespace net = boost::asio;                // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

void PrintData(const std::string& data) {
    HANDLE hPrinter;
    DOC_INFO_1W docInfo;
    DWORD dwBytesWritten;

    // Obtener el nombre de la impresora predeterminada
    wchar_t printerName[256];
    DWORD size = sizeof(printerName) / sizeof(printerName[0]);

    if (!GetDefaultPrinterW(printerName, &size)) {
        std::wcerr << L"Error al obtener el nombre de la impresora predeterminada." << std::endl;
        return;
    }

    // Abrir la impresora predeterminada
    if (!OpenPrinterW(printerName, &hPrinter, NULL)) {
        std::wcerr << L"Error al abrir la impresora." << std::endl;
        return;
    }

    wchar_t docName[] = L"Documento desde WebSocket";
    wchar_t dataType[] = L"RAW";

    docInfo.pDocName = docName;
    docInfo.pOutputFile = NULL;
    docInfo.pDatatype = dataType;

    // Iniciar el trabajo de impresión
    if (StartDocPrinterW(hPrinter, 1, (LPBYTE)&docInfo)) {
        if (StartPagePrinter(hPrinter)) {
            WritePrinter(hPrinter, (LPVOID)data.c_str(), static_cast<DWORD>(data.length()), &dwBytesWritten);
            EndPagePrinter(hPrinter);
        }
        EndDocPrinter(hPrinter);
    }
    else {
        std::wcerr << L"Error al iniciar la impresión." << std::endl;
    }

    ClosePrinter(hPrinter);
}

void StartWebSocketServer() {
    try {
        net::io_context ioc;
        tcp::acceptor acceptor{ ioc, tcp::endpoint{tcp::v4(), 1315} };

        std::wcout << L"Servidor WebSocket iniciado en el puerto 1315..." << std::endl;

        while (true) {
            tcp::socket socket{ ioc };
            acceptor.accept(socket);

            websocket::stream<tcp::socket> ws{ std::move(socket) };
            ws.accept();

            beast::flat_buffer buffer;
            ws.read(buffer);

            std::string data = beast::buffers_to_string(buffer.data());
            std::wcout << L"Recibido: " << data.c_str() << std::endl;
            PrintData(data);

            // Echo de vuelta al cliente (opcional)
            ws.text(ws.got_text());
            ws.write(buffer.data());
        }
    }
    catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    StartWebSocketServer();
    return 0;
}
