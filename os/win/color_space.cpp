// LAF OS Library
// Copyright (C) 2018  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "os/osx/color_space.h"

#include "base/file_content.h"
#include "base/fs.h"
#include "base/string.h"
#include "os/system.h"

#include <windows.h>

namespace os {

std::string get_hmonitor_icc_filename(HMONITOR monitor)
{
  std::string iccFilename;
  MONITORINFOEX mi;
  ZeroMemory(&mi, sizeof(MONITORINFOEX));
  mi.cbSize = sizeof(MONITORINFOEX);
  GetMonitorInfo(monitor, &mi);
  HDC hdc = CreateDC(mi.szDevice, nullptr, nullptr, nullptr);
  if (hdc) {
    DWORD length = MAX_PATH;
    std::vector<TCHAR> str(length+1);
    if (GetICMProfile(hdc, &length, &str[0]))
      iccFilename = base::to_utf8(&str[0]);
    DeleteDC(hdc);
  }
  return iccFilename;
}

os::ColorSpacePtr get_colorspace_from_icc_file(const std::string& iccFilename)
{
  auto buf = base::read_file_content(iccFilename);
  auto osCS = os::instance()->createColorSpace(gfx::ColorSpace::MakeICC(std::move(buf)));
  if (osCS) {
    osCS->gfxColorSpace()
      ->setName("Display Profile: " +
                base::get_file_title(iccFilename));
  }
  return osCS;
}

os::ColorSpacePtr get_hmonitor_colorspace(HMONITOR monitor)
{
  os::ColorSpacePtr osCS;
  std::string filename = get_hmonitor_icc_filename(monitor);
  if (!filename.empty())
    osCS = get_colorspace_from_icc_file(filename);
  return osCS;
}

static BOOL list_display_colorspaces_enumproc(HMONITOR monitor,
                                              HDC hdc, LPRECT rc,
                                              LPARAM data)
{
  auto list = (std::vector<os::ColorSpacePtr>*)data;
  auto osCS = get_hmonitor_colorspace(monitor);
  if (osCS)
    list->push_back(osCS);
  return TRUE;
}

void list_display_colorspaces(std::vector<os::ColorSpacePtr>& list)
{
  EnumDisplayMonitors(
    nullptr, nullptr,
    list_display_colorspaces_enumproc,
    (LPARAM)&list);
}

} // namespace os
