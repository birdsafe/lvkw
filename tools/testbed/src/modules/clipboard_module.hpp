#pragma once

#include "../feature_module.hpp"
#include <string>
#include <vector>

class ClipboardModule : public FeatureModule {
public:
  ClipboardModule();
  virtual ~ClipboardModule() = default;

  void update(lvkw::Context &ctx, lvkw::Window &window) override;
  void render(lvkw::Context &ctx, lvkw::Window &window) override;

  const char *getName() const override { return "Clipboard"; }
  bool &getEnabled() override { return enabled_; }

private:
  bool enabled_ = false;
  std::string clipboard_text_;
  std::vector<std::string> mime_types_;
  
  char input_buffer_[1024] = "";
};
