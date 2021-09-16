#==============================================================================
# Copyright (C) 2019-present Alces Flight Ltd.
#
# This file is part of Flight Slurm.
#
# This program and the accompanying materials are made available under
# the terms of the Eclipse Public License 2.0 which is available at
# <https://www.eclipse.org/legal/epl-2.0>, or alternative license
# terms made available by Alces Flight Ltd - please direct inquiries
# about licensing to licensing@alces-flight.com.
#
# Flight Slurm is distributed in the hope that it will be useful, but
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR
# IMPLIED INCLUDING, WITHOUT LIMITATION, ANY WARRANTIES OR CONDITIONS
# OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A
# PARTICULAR PURPOSE. See the Eclipse Public License 2.0 for more
# details.
#
# You should have received a copy of the Eclipse Public License 2.0
# along with Flight Slurm. If not, see:
#
#  https://opensource.org/licenses/EPL-2.0
#
# For more information on <project>, please visit:
# https://github.com/openflighthpc/slurm
#==============================================================================
export PATH=$PATH:$flight_ROOT/opt/slurm/bin

if [ -n "$flight_DEFINES_paths" ]; then
  flight_DEFINES_paths="$flight_DEFINES_paths $flight_ROOT/opt/slurm/bin"
fi
