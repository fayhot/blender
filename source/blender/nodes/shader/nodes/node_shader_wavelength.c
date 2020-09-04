/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2005 Blender Foundation.
 * All rights reserved.
 */

#include "../node_shader_util.h"

#include "BKE_spectral_stuff.h"

/* **************** Wavelength ******************** */
static bNodeSocketTemplate sh_node_wavelength_in[] = {
    {SOCK_FLOAT, N_("Wavelength"), 500.0f, 0.0f, 0.0f, 0.0f, MIN_WAVELENGTH, MAX_WAVELENGTH},
    {-1, ""},
};

static bNodeSocketTemplate sh_node_wavelength_out[] = {
    {SOCK_RGBA, N_("Color")},
    {-1, ""},
};

/* node type definition */
void register_node_type_sh_wavelength(void)
{
  static bNodeType ntype;

  sh_node_type_base(&ntype, SH_NODE_WAVELENGTH, "Wavelength", NODE_CLASS_CONVERTOR, 0);
  node_type_size_preset(&ntype, NODE_SIZE_MIDDLE);
  node_type_socket_templates(&ntype, sh_node_wavelength_in, sh_node_wavelength_out);
  node_type_init(&ntype, NULL);
  node_type_storage(&ntype, "", NULL, NULL);

  nodeRegisterType(&ntype);
}
