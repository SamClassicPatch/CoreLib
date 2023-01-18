/* Copyright (c) 2022-2023 Dreamy Cecil
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef CECIL_INCL_GFXFUNCTIONS_H
#define CECIL_INCL_GFXFUNCTIONS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Engine/Graphics/GfxLibrary.h>

// Interface of public methods from the engine's graphical API
class IGfx {
  public:
    #if SE1_VER >= SE1_107
      // Pointers to function pointers
      #define GFX_FUNC_POINTER **
    #else
      // Pointers to functions
      #define GFX_FUNC_POINTER *
    #endif

    typedef void (GFX_FUNC_POINTER CVoidFunc   )(void);
    typedef void (GFX_FUNC_POINTER CBlendFunc  )(GfxBlend, GfxBlend);
    typedef void (GFX_FUNC_POINTER CDepthFunc  )(GfxComp);
    typedef void (GFX_FUNC_POINTER CFaceFunc   )(GfxFace);
    typedef void (GFX_FUNC_POINTER COrthoFunc  )(FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, BOOL);
    typedef void (GFX_FUNC_POINTER CFrustumFunc)(FLOAT, FLOAT, FLOAT, FLOAT, FLOAT, FLOAT);
    typedef void (GFX_FUNC_POINTER CPolModeFunc)(GfxPolyMode);
    typedef void (GFX_FUNC_POINTER CColorsFunc )(ULONG);
    typedef void (GFX_FUNC_POINTER CTexRefFunc )(ULONG &);
    typedef void (GFX_FUNC_POINTER CMatrixFunc )(const FLOAT *);
    typedef void (GFX_FUNC_POINTER CClipPlFunc )(const DOUBLE *);
    typedef void (GFX_FUNC_POINTER CSetVtxFunc )(GFXVertex4 *, INDEX);
    typedef void (GFX_FUNC_POINTER CSetNorFunc )(GFXNormal *);
    typedef void (GFX_FUNC_POINTER CSetCrdFunc )(GFXTexCoord *, BOOL);
    typedef void (GFX_FUNC_POINTER CSetColFunc )(GFXColor *);
    typedef void (GFX_FUNC_POINTER CDrawElFunc )(INDEX, INDEX *);
    typedef void (GFX_FUNC_POINTER CWrapUVFunc )(GfxWrap, GfxWrap);
    typedef void (GFX_FUNC_POINTER CTexModFunc )(INDEX);
    typedef void (GFX_FUNC_POINTER CMinMaxFunc )(FLOAT, FLOAT);

    #define GFX_FUNC static __forceinline void

    #define ARGS_GfxSetOrtho   (FLOAT fL, FLOAT fR, FLOAT fT, FLOAT fB, FLOAT fNear, FLOAT fFar, BOOL bSubPixelAdjust)
    #define ARGS_GfxSetFrustum (FLOAT fL, FLOAT fR, FLOAT fT, FLOAT fB, FLOAT fNear, FLOAT fFar)

  // [Cecil] TODO: Find addresses of methods in both 1.05 games
  #if SE1_VER == SE1_107
  // The same order as in GFX_SetGraphicsPointers()
  public:

    GFX_FUNC EnableDepthWrite(void)                     { (*(CVoidFunc   )0x602BBB64)(); };
    GFX_FUNC EnableDepthBias(void)                      { (*(CVoidFunc   )0x602BBB68)(); };
    GFX_FUNC EnableDepthTest(void)                      { (*(CVoidFunc   )0x602BBB6C)(); };
    GFX_FUNC EnableAlphaTest(void)                      { (*(CVoidFunc   )0x602BBB70)(); };
    GFX_FUNC EnableBlend(void)                          { (*(CVoidFunc   )0x602BBB74)(); };
    GFX_FUNC EnableDither(void)                         { (*(CVoidFunc   )0x602BBB78)(); };
    GFX_FUNC EnableTexture(void)                        { (*(CVoidFunc   )0x602BBB7C)(); };
    GFX_FUNC EnableClipping(void)                       { (*(CVoidFunc   )0x602BBB80)(); };
    GFX_FUNC EnableClipPlane(void)                      { (*(CVoidFunc   )0x602BBB84)(); };
    GFX_FUNC EnableTruform(void)                        { (*(CVoidFunc   )0x602BBC10)(); };
    GFX_FUNC DisableDepthWrite(void)                    { (*(CVoidFunc   )0x602BBB88)(); };
    GFX_FUNC DisableDepthBias(void)                     { (*(CVoidFunc   )0x602BBB8C)(); };
    GFX_FUNC DisableDepthTest(void)                     { (*(CVoidFunc   )0x602BBB90)(); };
    GFX_FUNC DisableAlphaTest(void)                     { (*(CVoidFunc   )0x602BBB94)(); };
    GFX_FUNC DisableBlend(void)                         { (*(CVoidFunc   )0x602BBB98)(); };
    GFX_FUNC DisableDither(void)                        { (*(CVoidFunc   )0x602BBB9C)(); };
    GFX_FUNC DisableTexture(void)                       { (*(CVoidFunc   )0x602BBBA0)(); };
    GFX_FUNC DisableClipping(void)                      { (*(CVoidFunc   )0x602BBBA4)(); };
    GFX_FUNC DisableClipPlane(void)                     { (*(CVoidFunc   )0x602BBBA8)(); };
    GFX_FUNC DisableTruform(void)                       { (*(CVoidFunc   )0x602BBC14)(); };
    GFX_FUNC BlendFunc(GfxBlend eSrc, GfxBlend eDst)    { (*(CBlendFunc  )0x602BBBAC)(eSrc, eDst); };
    GFX_FUNC DepthFunc(GfxComp eComp)                   { (*(CDepthFunc  )0x602BBBB0)(eComp); };
    GFX_FUNC DepthRange(FLOAT fMin, FLOAT fMax)         { (*(CMinMaxFunc )0x602BBBB4)(fMin, fMax); };
    GFX_FUNC CullFace(GfxFace eFace)                    { (*(CFaceFunc   )0x602BBBB8)(eFace); };
    GFX_FUNC FrontFace(GfxFace eFace)                   { (*(CFaceFunc   )0x602BBBBC)(eFace); };
    GFX_FUNC ClipPlane(const DOUBLE *pdViewPlane)       { (*(CClipPlFunc )0x602BBBC0)(pdViewPlane); };
    GFX_FUNC SetOrtho   ARGS_GfxSetOrtho                { (*(COrthoFunc  )0x602BBBC4)(fL, fR, fT, fB, fNear, fFar, bSubPixelAdjust); };
    GFX_FUNC SetFrustum ARGS_GfxSetFrustum              { (*(CFrustumFunc)0x602BBBC8)(fL, fR, fT, fB, fNear, fFar); };
    GFX_FUNC SetTextureMatrix(const FLOAT *pfMatrix)    { (*(CMatrixFunc )0x602BBBCC)(pfMatrix); };
    GFX_FUNC SetViewMatrix(const FLOAT *pfMatrix)       { (*(CMatrixFunc )0x602BBBD0)(pfMatrix); };
    GFX_FUNC PolygonMode(GfxPolyMode eMode)             { (*(CPolModeFunc)0x602BBBD4)(eMode); };
    GFX_FUNC SetTextureWrapping(GfxWrap eU, GfxWrap eV) { (*(CWrapUVFunc )0x602BBBD8)(eU, eV); };
    GFX_FUNC SetTextureModulation(INDEX i)              { (*(CTexModFunc )0x602BBBDC)(i); };
    GFX_FUNC GenerateTexture(ULONG &ulTex)              { (*(CTexRefFunc )0x602BBBE0)(ulTex); };
    GFX_FUNC DeleteTexture(ULONG &ulTex)                { (*(CTexRefFunc )0x602BBBE4)(ulTex); };
    GFX_FUNC SetVertexArray(GFXVertex4 *aVtx, INDEX ct) { (*(CSetVtxFunc )0x602BBBE8)(aVtx, ct); };
    GFX_FUNC SetNormalArray(GFXNormal *aNot)            { (*(CSetNorFunc )0x602BBBEC)(aNot); };
    GFX_FUNC SetTexCoordArray(GFXTexCoord *a, BOOL b4)  { (*(CSetCrdFunc )0x602BBBF0)(a, b4); };
    GFX_FUNC SetColorArray(GFXColor *aCol)              { (*(CSetColFunc )0x602BBBF4)(aCol); };
    GFX_FUNC DrawElements(INDEX ct, INDEX *aElements)   { (*(CDrawElFunc )0x602BBBF8)(ct, aElements); };
    GFX_FUNC SetConstantColor(ULONG ulCol)              { (*(CColorsFunc )0x602BBBFC)(ulCol); };
    GFX_FUNC EnableColorArray(void)                     { (*(CVoidFunc   )0x602BBC00)(); };
    GFX_FUNC DisableColorArray(void)                    { (*(CVoidFunc   )0x602BBC04)(); };
    GFX_FUNC Finish(void)                               { (*(CVoidFunc   )0x602BBC08)(); };
    GFX_FUNC LockArrays(void)                           { (*(CVoidFunc   )0x602BBC0C)(); };
    GFX_FUNC SetColorMask(ULONG ulCol)                  { (*(CColorsFunc )0x602BBC18)(ulCol); };

  // Non-pointer functions
  public:

    GFX_FUNC GetTextureFiltering(INDEX &iFilterType, INDEX &iAnisotropyDegree) {
      ((void (*)(INDEX &, INDEX &))0x6011B0F0)(iFilterType, iAnisotropyDegree);
    };

    GFX_FUNC SetTextureFiltering(INDEX &iFilterType, INDEX &iAnisotropyDegree) {
      ((void (*)(INDEX &, INDEX &))0x6011B110)(iFilterType, iAnisotropyDegree);
    };

    GFX_FUNC SetTextureBiasing(FLOAT &fLODBias) {
      ((void (*)(FLOAT &))0x6011B300)(fLODBias);
    };

    GFX_FUNC SetTextureUnit(INDEX iUnit) {
      ((void (*)(INDEX))0x6011B380)(iUnit);
    };

    GFX_FUNC SetTexture(ULONG &ulTex, CTexParams &tpLocal) {
      ((void (*)(ULONG &, CTexParams &))0x6011B420)(ulTex, tpLocal);
    };

    GFX_FUNC UploadTexture(ULONG *pulTex, PIX pixW, PIX pixH, ULONG ulFormat, BOOL bNoDiscard) {
      ((void (*)(ULONG *, PIX, PIX, ULONG, BOOL))0x6011B5F0)(pulTex, pixW, pixH, ulFormat, bNoDiscard);
    };

    GFX_FUNC UnlockArrays(void)  { ((void (*)(void))0x6011C080)(); };
    GFX_FUNC FlushElements(void) { ((void (*)(void))0x6011BEC0)(); };
    GFX_FUNC FlushQuads(void)    { ((void (*)(void))0x6011BD90)(); };

  #endif
};

#endif
