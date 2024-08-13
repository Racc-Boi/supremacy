#pragma once

#pragma warning( disable : 4307 ) // '*': integral constant overflow
#pragma warning( disable : 4244 ) // possible loss of data
#pragma warning( disable : 4800 ) // forcing value to bool 'true' or 'false'
#pragma warning( disable : 4838 ) // conversion from '::size_t' to 'int' requires a narrowing conversion

// You can define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING or _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS to acknowledge that you have received this warning.
#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

using ulong_t = unsigned long;

// windows / stl includes.
#include <Windows.h>
#include <cstdint>
#include <intrin.h>
#include <xmmintrin.h>
#include <array>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>
#include <chrono>
#include <thread>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <deque>
#include <functional>
#include <map>
#include <shlobj.h>
#include <filesystem>
#include <streambuf>

// our custom wrapper.
#include "util/unique_vector.h"
#include "util/contrib/tinyformat.h"

// other includes.
#include "util/hash.h"
#include "util/xorstr.h"
#include "util/pe.h"
#include "util/winapir.h"
#include "util/address.h"
#include "util/util.h"
#include "util/modules.h"
#include "util/pattern.h"
#include "util/vmt.h"
#include "util/stack.h"
#include "util/nt.h"
#include "util/x86.h"
#include "util/syscall.h"
#include "util/minhook/minhook.h"
#include "util/detourhook.h"

// hack includes.
#include "core/csgo/interfaces.h"
#include "sdk/sdk.h"
#include "core/csgo/csgo.h"
#include "features/penetration/penetration.h"
#include "core/csgo/netvars.h"
#include "sdk/entoffsets.h"
#include "sdk/entity.h"
#include "core/csgo/client.h"
#include "sdk/gamerules.h"
#include "core/hooks/hooks.h"
#include "core/hooks/detours.h"
#include "util/render.h"
#include "features/prediction/pred.h"
#include "features/lagcomp/lagrecord.h"
#include "features/visuals/visuals.h"
#include "features/movement/movement.h"
#include "features/bones/bonesetup.h"
#include "features/antiaim/hvh.h"
#include "features/lagcomp/lagcomp.h"
#include "features/aim/aimbot.h"
#include "features/network/netdata.h"
#include "features/chams/chams.h"
#include "features/visuals/notify.h"
#include "features/resolver/resolver.h"
#include "features/visuals/grenades.h"
#include "features/skins/skins.h"
#include "features/events/events.h"
#include "features/shots/shots.h"

// gui includes.
#include "util/contrib/json.h"
#include "util/contrib/base64.h"
#include "core/gui/elements/element.h"
#include "core/gui/elements/checkbox.h"
#include "core/gui/elements/dropdown.h"
#include "core/gui/elements/multidropdown.h"
#include "core/gui/elements/slider.h"
#include "core/gui/elements/colorpicker.h"
#include "core/gui/elements/edit.h"
#include "core/gui/elements/keybind.h"
#include "core/gui/elements/button.h"
#include "core/gui/elements/tab.h"
#include "core/gui/elements/form.h"
#include "core/gui/gui.h"
#include "core/gui/elements/callbacks.h"
#include "core/gui/menu.h"
#include "core/config/config.h"