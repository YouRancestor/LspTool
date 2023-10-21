#ifndef LSPSERVERPROXY_H
#define LSPSERVERPROXY_H

#include <Windows.h>
#include <string>

typedef void (*FunOnMsgRecv)(const char* data, size_t len, void* opaque);
typedef void (*FunOnSvrExit)(int exit_code, void* opaque);

/**
 *
 */
class LspServerProxy
{
public:
    enum State
    {
        OK = 0,
    };

    LspServerProxy(const char* Exe, const char* Args);
    LspServerProxy(const char* Cmd);
    ~LspServerProxy();

    int Init();
    void Clean();

    inline int GetState() { return ErrCode; }

    void SendCommand(std::string CmdStr);
    void SendRaw(const char* buff, DWORD len);

    void SetCallback(FunOnMsgRecv OnMsgRecv, void* Opaque);
    void SetCallback(FunOnSvrExit OnSvrExit, void* Opaque);

private:
    void OnDataRecv(const char* data, size_t len);
    friend DWORD WINAPI ReadProc(LPVOID lpParam);
    friend DWORD WINAPI WaitProc(LPVOID lpParam);
    int ErrCode;
    HANDLE InputRead;
    HANDLE InputWrite;
    HANDLE OutputRead;
    HANDLE OutputWrite;
    HANDLE ReadThread;
    PROCESS_INFORMATION pi;
    HANDLE WaitThread;

    std::string BuffRecv;

    FunOnMsgRecv MsgRecvCb;
    void* MsgRecvCbOpaque;
    FunOnSvrExit LeanExitCb;
    void* LeanExitCbOpaque;

    std::string CmdWithArgs;
};

#endif // LSPSERVERPROXY_H
