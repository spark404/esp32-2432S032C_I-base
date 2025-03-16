#
# SPDX-FileCopyrightText: 2025 Hugo Trippaers
#
# SPDX-License-Identifier: Apache-2.0
#

idf_component_register(
        SRCS
          "esp32-2432S032C_I-base.c"
          "pretty_effect.c"
          "decode_image.c"
        INCLUDE_DIRS
          "."
        EMBED_FILES
          "image.jpg"
)
