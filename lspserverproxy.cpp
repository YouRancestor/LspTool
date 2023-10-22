#include "lspserverproxy.h"


#include "lspserverproxy.h"

#define BUFF_LEN 204800 // TODO: 提高buff长度
DWORD WINAPI ReadProc(LPVOID lpParam)
{
    LspServerProxy* self = (LspServerProxy*)lpParam;
    char buff[BUFF_LEN];
    HANDLE hOutputRead = self->OutputRead;
    DWORD ReadNum;
    while (true)
    {
        BOOL ret = ReadFile(hOutputRead, buff, BUFF_LEN, &ReadNum, NULL);
        if (ret)
        {
            self->OnDataRecv(buff, ReadNum);
        }
    }
    return 0;
}

DWORD WINAPI WaitProc(LPVOID lpParam)
{
    LspServerProxy* self = (LspServerProxy*)lpParam;
    WaitForSingleObject(self->pi.hProcess, INFINITE);
    self->ErrCode = 4;
    DWORD exit_code;
    BOOL ret = GetExitCodeProcess(self->pi.hProcess, &exit_code);
    if (ret)
    {
        if (exit_code != STILL_ACTIVE)
        {
            self->Clean();
            if (self->LeanExitCb)
                self->LeanExitCb(exit_code, self->LeanExitCbOpaque);
        }
    }
    else
    {
        DWORD error = GetLastError();
        char buff[64];
        sprintf_s(buff, "Error with code: %d.", int(error));
        MessageBoxA(NULL, buff, "error", MB_OK);
        exit(-1);
    }
    return 0;
}

LspServerProxy::LspServerProxy(const char* Exe, const char* Args)
    : LspServerProxy(NULL)
{
    CmdWithArgs = std::string(Exe) + " " + Args;
}

LspServerProxy::LspServerProxy(const char *Cmd)
    : ErrCode(1)
    , InputRead(NULL)
    , InputWrite(NULL)
    , OutputRead(NULL)
    , OutputWrite(NULL)
    , ReadThread(NULL)
    , pi({ 0 })
    , WaitThread(NULL)
    , MsgRecvCb(NULL)
    , MsgRecvCbOpaque(NULL)
    , LeanExitCb(NULL)
    , LeanExitCbOpaque(NULL)
    , CmdWithArgs(Cmd)
{

}

LspServerProxy::~LspServerProxy()
{
    Clean();
}

int LspServerProxy::Init()
{
    // 创建管道，用于接收Lean的标准输出
    SECURITY_ATTRIBUTES sa = { 0 };
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    if (!CreatePipe(&OutputRead, &OutputWrite, &sa, BUFF_LEN))
    {
        ErrCode = 1;
        return ErrCode;
    }
    // 创建管道，用于将数据发送到Lean的标准输入
    if (!CreatePipe(&InputRead, &InputWrite, &sa, BUFF_LEN))
    {
        ErrCode = 2;
        return ErrCode;
    }
    // 创建读线程，接收Lean输出的数据
    DWORD ReadThreadID;
    ReadThread = CreateThread(NULL, 0, ReadProc, this, 0, &ReadThreadID);
    if (!ReadThread)
    {
        ErrCode = 3;
        return ErrCode;
    }
    // 创建进程，以服务模式启动Lean
    STARTUPINFOA si = { 0 };
    si.cb = sizeof(STARTUPINFOA);
    GetStartupInfoA(&si);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdInput = InputRead;
    si.hStdError = OutputWrite;
    si.hStdOutput = OutputWrite;
    //char arg[]="lean4.exe --version";
    //BOOL result = CreateProcessA(NULL, arg, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi);
    Sleep(500); // 等读线程启动
    BOOL result = CreateProcessA(NULL, &CmdWithArgs[0], NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi);
    DWORD err = GetLastError();
    if (!result)
    {
        ErrCode = 4;
        return ErrCode;
    }
    // 创建等待线程，等待Lean进程退出
    DWORD WaitThreadID;
    WaitThread = CreateThread(NULL, 0, WaitProc, this, 0, &WaitThreadID);
    if (!WaitThread)
    {
        ErrCode = 5;
        return ErrCode;
    }
    ErrCode = 0;
    return ErrCode;
}

void LspServerProxy::Clean()
{
    switch (ErrCode)
    {
    case 0:
    case 5:
        // 结束Lean进程
        TerminateProcess(pi.hProcess, 0);
        WaitForSingleObject(pi.hProcess, INFINITE);
        WaitForSingleObject(WaitThread, INFINITE);
    case 4:
        // 结束读线程
        TerminateThread(ReadThread, 0);
        WaitForSingleObject(ReadThread, INFINITE);
    case 3:
        // 关闭输入管道
        CloseHandle(InputRead);
        CloseHandle(InputWrite);
    case 2:
        // 关闭输出管道
        CloseHandle(OutputRead);
        CloseHandle(OutputWrite);
    case 1:
    default:
        ErrCode = 1;
        break;
    }
}

void LspServerProxy::SendCommand(std::string CmdStr)
{
    std::string& buff = CmdStr;
    size_t len = buff.length();
    DWORD TotalWrite = 0;
    DWORD NumWritten;
    do
    {
        WriteFile(InputWrite, buff.c_str() + TotalWrite, (len - TotalWrite) * sizeof(char), &NumWritten, NULL);
        TotalWrite += NumWritten;

    } while (TotalWrite < len);
}

void LspServerProxy::SendRaw(const char* buff, DWORD len)
{
    DWORD TotalWrite = 0;
    DWORD NumWritten;
    do
    {
        DWORD ToWrite = len - TotalWrite;
        WriteFile(InputWrite, buff + TotalWrite, ToWrite * sizeof(char), &NumWritten, NULL);
        TotalWrite += NumWritten;
    } while (TotalWrite < len);
}

void LspServerProxy::SetCallback(FunOnMsgRecv OnMsgRecv, void* Opaque)
{
    MsgRecvCb = OnMsgRecv;
    MsgRecvCbOpaque = Opaque;
}

void LspServerProxy::SetCallback(FunOnSvrExit OnSvrExit, void* Opaque)
{
    LeanExitCb = OnSvrExit;
    LeanExitCbOpaque = Opaque;
}

void LspServerProxy::OnDataRecv(const char* data, size_t len)
{
    if (MsgRecvCb)
    {
        MsgRecvCb(data, len, MsgRecvCbOpaque);
    }
}
