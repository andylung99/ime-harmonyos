#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "napi/native_api.h"

#include "EngineCore.h"
#include "WubiDict.h"

namespace {

ime::EngineCore& Core() {
  static ime::EngineCore core;
  return core;
}

ime::WubiDict& Wubi() {
  static ime::WubiDict wubi;
  return wubi;
}

napi_value StrListToNapi(napi_env env, const std::vector<std::string>& items) {
  napi_value arr;
  napi_create_array_with_length(env, items.size(), &arr);
  for (size_t i = 0; i < items.size(); ++i) {
    napi_value s;
    napi_create_string_utf8(env, items[i].c_str(), items[i].size(), &s);
    napi_set_element(env, arr, static_cast<uint32_t>(i), s);
  }
  return arr;
}

bool GetStringArg(napi_env env, napi_value value, std::string* out) {
  size_t len = 0;
  napi_get_value_string_utf8(env, value, nullptr, 0, &len);
  std::string buf(len, '\0');
  napi_get_value_string_utf8(env, value, buf.data(), buf.size() + 1, &len);
  out->assign(buf.data(), len);
  return true;
}

napi_value Feed(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr, nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  double x = 0, y = 0;
  if (argc > 0) {
    napi_get_value_double(env, args[0], &x);
  }
  if (argc > 1) {
    napi_get_value_double(env, args[1], &y);
  }
  Core().Feed(static_cast<float>(x), static_cast<float>(y));
  napi_value result;
  napi_get_undefined(env, &result);
  return result;
}

napi_value Decode(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  uint32_t topN = 6;
  if (argc > 0) {
    napi_get_value_uint32(env, args[0], &topN);
  }
  return StrListToNapi(env, Core().Decode(static_cast<int>(topN)));
}

napi_value Backspace(napi_env env, napi_callback_info info) {
  Core().Backspace();
  return StrListToNapi(env, Core().Decode(6));
}

napi_value Reset(napi_env env, napi_callback_info info) {
  Core().Reset();
  napi_value result;
  napi_get_undefined(env, &result);
  return result;
}

napi_value Learn(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  std::string word;
  if (argc > 0) {
    GetStringArg(env, args[0], &word);
  }
  if (!word.empty()) {
    Core().Learn(word);
  }
  napi_value result;
  napi_get_undefined(env, &result);
  return result;
}

napi_value SetSigma(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  double sigma = 28;
  if (argc > 0) {
    napi_get_value_double(env, args[0], &sigma);
  }
  Core().SetSigma(static_cast<float>(sigma));
  napi_value result;
  napi_get_undefined(env, &result);
  return result;
}

napi_value ClearUserData(napi_env env, napi_callback_info info) {
  Core().ClearUserData();
  napi_value result;
  napi_get_undefined(env, &result);
  return result;
}

napi_value LoadWubi(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr, nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  std::string tab, py;
  if (argc > 0) {
    GetStringArg(env, args[0], &tab);
  }
  if (argc > 1) {
    GetStringArg(env, args[1], &py);
  }
  Wubi().Load(tab, py);
  napi_value result;
  napi_get_boolean(env, true, &result);
  return result;
}

napi_value WubiQuery(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr, nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  std::string code;
  uint32_t topN = 8;
  if (argc > 0) {
    GetStringArg(env, args[0], &code);
  }
  if (argc > 1) {
    napi_get_value_uint32(env, args[1], &topN);
  }
  std::vector<std::string> out;
  for (const auto& e : Wubi().Query(code, static_cast<int>(topN))) {
    out.push_back(e.word);
  }
  return StrListToNapi(env, out);
}

napi_value WubiReverse(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value args[2] = {nullptr, nullptr};
  napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  std::string pinyin;
  uint32_t topN = 8;
  if (argc > 0) {
    GetStringArg(env, args[0], &pinyin);
  }
  if (argc > 1) {
    napi_get_value_uint32(env, args[1], &topN);
  }
  std::vector<std::string> out;
  for (const auto& e : Wubi().ReverseByPinyin(pinyin, static_cast<int>(topN))) {
    out.push_back(e.word);
  }
  return StrListToNapi(env, out);
}

napi_value Init(napi_env env, napi_value exports) {
  napi_property_descriptor desc[] = {
      {"feed", nullptr, Feed, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"decode", nullptr, Decode, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"backspace", nullptr, Backspace, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"reset", nullptr, Reset, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"learn", nullptr, Learn, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"setSigma", nullptr, SetSigma, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"clearUserData", nullptr, ClearUserData, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"loadWubi", nullptr, LoadWubi, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"wubiQuery", nullptr, WubiQuery, nullptr, nullptr, nullptr, napi_default, nullptr},
      {"wubiReverse", nullptr, WubiReverse, nullptr, nullptr, nullptr, napi_default, nullptr},
  };
  napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
  return exports;
}

}  // namespace

static napi_module imeModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "engine",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterImeModule(void) {
  napi_module_register(&imeModule);
}