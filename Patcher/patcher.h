// patcher.h
#pragma once

#ifndef ___C_CPP_PATCHER_H___
#define ___C_CPP_PATCHER_H___

#include <windows.h>

#pragma warning(push)

#ifdef __DO_NOT_SHOW_PATCHER_WARNINGS__
  #pragma warning(disable : 4311)
#endif

class CPatch
{
  private:
    // Don't care about leaks, it is allocated only once
    static HANDLE s_hHeap;

  private:
    bool m_valid;
    bool m_patched;
    bool m_set_forever;
    long m_old_jmp;
    char* m_PatchInstructionSet;
    char* m_RestorePatchSet;
    int m_size;
    int m_restore_size;
    DWORD m_protect;
    long m_FuncToHook;

    // Default constructor (illegal)
    CPatch() {};

    // Copy constructor (illegal)
    CPatch(CPatch &) {};

  // [Cecil] Made public
  public:
    template<class T1, class T2> inline void HookClassFunctions(T1& fn_funcToHook, T2 fn_Hook, bool patch_now, bool set_forever)
    {
      //long& NewCallAddress( *reinterpret_cast<long*>(&fn_funcToHook)  );
      //long& NewCallAddress = (long&)(void*&)fn_funcToHook;
      T1* pT1 = &fn_funcToHook;
      long* ppT1 = reinterpret_cast<long*>(pT1);
      long& NewCallAddress = *ppT1;
      long  MyHook        ( *reinterpret_cast<long*>(&fn_Hook)        );
      HookFunction(NewCallAddress, MyHook, &NewCallAddress, patch_now);
    }

  protected:
    bool okToRewriteTragetInstructionSet(long addr, int& rw_len);
    BOOL HookFunction(long FuncToHook, long  MyHook, long* NewCallAddress, bool patch_now = true);

  public:
    template<class TFunction>explicit CPatch(TFunction FuncToHook, TFunction MyHook, TFunction& NewCallAddress, bool patch_now = true, bool set_forever = false)
                  : m_valid(false)
                  , m_patched(false)
                  , m_set_forever(set_forever)
                  , m_PatchInstructionSet(0)
                  , m_RestorePatchSet(0)
    {
      HookFunction(reinterpret_cast<long>(FuncToHook), reinterpret_cast<long>(MyHook), reinterpret_cast<long*>(&NewCallAddress), patch_now);
    };

    template<class TFunction>explicit CPatch(TFunction FuncToHook, TFunction MyHook, TFunction* NewCallAddress, bool patch_now = true, bool set_forever = false)
                  : m_valid(false)
                  , m_patched(false)
                  , m_set_forever(set_forever)
                  , m_PatchInstructionSet(0)
                  , m_RestorePatchSet(0)
    {
      HookFunction(reinterpret_cast<long>(FuncToHook), reinterpret_cast<long>(MyHook), reinterpret_cast<long*>(NewCallAddress), patch_now);
    };

    template<class TFunction>explicit CPatch(TFunction& NewCallAddress, TFunction MyHook, bool patch_now = true, bool set_forever = false)
                  : m_valid(false)
                  , m_patched(false)
                  , m_set_forever(set_forever)
                  , m_PatchInstructionSet(0)
                  , m_RestorePatchSet(0)
    {
      HookFunction(reinterpret_cast<long>(NewCallAddress), reinterpret_cast<long>(MyHook), reinterpret_cast<long*>(&NewCallAddress), patch_now);
    };

    template<class TFunction>explicit CPatch(TFunction* NewCallAddress, TFunction MyHook, bool patch_now = true, bool set_forever = false)
                  : m_valid(false)
                  , m_patched(false)
                  , m_set_forever(set_forever)
                  , m_PatchInstructionSet(0)
                  , m_RestorePatchSet(0)
    {
      HookFunction(reinterpret_cast<long>(*NewCallAddress), reinterpret_cast<long>(MyHook), reinterpret_cast<long*>(*NewCallAddress), patch_now);
    };


    #define ____C_CPP_PATCHER_DEFINISIONS_INCL____
    #include "patcher_defines.h"

    // Destructor
    ~CPatch();

    // Check if patch has been set
    bool patched(void);

    // Check if the patch is valid
    bool ok();

    // Set patch validity
    bool ok(bool _valid);

    // Restore old function
    void remove_patch(bool forever = false);

    // Set new function
    void set_patch(void);

  // [Cecil] Extensions
  public:
    static bool _bDebugOutput; // Display debug output
    static CTString _strPatcherLog; // Information to display

    // Allowed to rewrite anything of this length
    static int _iRewriteLen;

  public:
    // Constructor without immediate function hooking
    CPatch(bool bSetForever) :
      m_valid(false), m_patched(false), m_set_forever(bSetForever),
      m_PatchInstructionSet(0), m_RestorePatchSet(0)
    {
    };

    // Patcher debug output
    static inline bool &DebugOutput(void) {
      return _bDebugOutput;
    };

    // Force instruction rewrite
    static inline void ForceRewrite(const int iLength) {
      _iRewriteLen = iLength;
    };
};

#pragma warning(pop)

#endif
