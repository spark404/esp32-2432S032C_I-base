#
# SPDX-FileCopyrightText: 2025 Hugo Trippaers
#
# SPDX-License-Identifier: Apache-2.0
#

idf_component_register(
        SRCS
          "esp32-2432S032C_I-base.c"
          "openart-image_320x240.c"
          "ui.c"
          "."
        EMBED_FILES
          "image.jpg"
)
