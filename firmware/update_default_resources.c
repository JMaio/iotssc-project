// ----------------------------------------------------------------------------
// Copyright 2019 ARM Limited or its affiliates
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                   Autogenerated file - do not edit
//
//  generated on: 2021-02-12 16:37:47 GMT Standard Time
//  generated by: pdmfota-dev init
//  tool version: 1.7
//-----------------------------------------------------------------------------

#include <inttypes.h>

const uint8_t arm_uc_vendor_id[] = {
    0xA7, 0x1E, 0xC7, 0x81, 0xC8, 0xB2, 0x4F, 0x59,
    0x94, 0xBA, 0xFF, 0x48, 0xD9, 0x7C, 0x4D, 0x9B
};
const uint16_t arm_uc_vendor_id_size = sizeof(arm_uc_vendor_id);

const uint8_t arm_uc_class_id[] = {
    0xE3, 0x99, 0x39, 0x4C, 0x88, 0xBC, 0x4C, 0xA8,
    0xA2, 0xBD, 0x2F, 0xB1, 0x84, 0x46, 0x82, 0xB9
};
const uint16_t arm_uc_class_id_size = sizeof(arm_uc_class_id);

// TODO: remove it when no longer used
// This value is here for backwards compatability purposes - it is not used
const uint8_t arm_uc_default_fingerprint[] =  {
    0
};
const uint16_t arm_uc_default_fingerprint_size = sizeof(arm_uc_default_fingerprint);

const uint8_t arm_uc_default_certificate[] = {
    0x30, 0x82, 0x01, 0x83, 0x30, 0x82, 0x01, 0x29,
    0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x14, 0x19,
    0x14, 0x7D, 0x0A, 0x1D, 0x03, 0x45, 0xD4, 0xAE,
    0xE4, 0x87, 0x67, 0x11, 0x6D, 0xA1, 0x5E, 0xE9,
    0xA0, 0x6C, 0x32, 0x30, 0x0A, 0x06, 0x08, 0x2A,
    0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x30,
    0x14, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55,
    0x04, 0x03, 0x0C, 0x09, 0x6C, 0x6F, 0x63, 0x61,
    0x6C, 0x68, 0x6F, 0x73, 0x74, 0x30, 0x1E, 0x17,
    0x0D, 0x32, 0x31, 0x30, 0x32, 0x31, 0x32, 0x31,
    0x36, 0x33, 0x37, 0x34, 0x37, 0x5A, 0x17, 0x0D,
    0x32, 0x32, 0x30, 0x32, 0x31, 0x32, 0x31, 0x36,
    0x33, 0x37, 0x34, 0x37, 0x5A, 0x30, 0x14, 0x31,
    0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x03,
    0x0C, 0x09, 0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x68,
    0x6F, 0x73, 0x74, 0x30, 0x59, 0x30, 0x13, 0x06,
    0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
    0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03,
    0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x67, 0x84,
    0xC1, 0xEF, 0x01, 0x05, 0xE9, 0x82, 0xD2, 0xBD,
    0x26, 0xFE, 0x9E, 0x50, 0x6B, 0x6A, 0x6C, 0x58,
    0x16, 0x46, 0xB5, 0x4A, 0x9A, 0x3F, 0x52, 0xDD,
    0x31, 0x3F, 0x04, 0xD6, 0x31, 0x97, 0x67, 0x34,
    0xA2, 0xE7, 0xB7, 0x61, 0xA6, 0xB1, 0x06, 0x68,
    0xA9, 0x8C, 0x0D, 0xBD, 0x95, 0x6C, 0x42, 0xDA,
    0xC0, 0x51, 0x25, 0xE9, 0xD6, 0x85, 0x1B, 0xCC,
    0xBA, 0xB4, 0xF6, 0x6D, 0x8A, 0xB8, 0xA3, 0x59,
    0x30, 0x57, 0x30, 0x0B, 0x06, 0x03, 0x55, 0x1D,
    0x0F, 0x04, 0x04, 0x03, 0x02, 0x07, 0x80, 0x30,
    0x14, 0x06, 0x03, 0x55, 0x1D, 0x11, 0x04, 0x0D,
    0x30, 0x0B, 0x82, 0x09, 0x6C, 0x6F, 0x63, 0x61,
    0x6C, 0x68, 0x6F, 0x73, 0x74, 0x30, 0x13, 0x06,
    0x03, 0x55, 0x1D, 0x25, 0x04, 0x0C, 0x30, 0x0A,
    0x06, 0x08, 0x2B, 0x06, 0x01, 0x05, 0x05, 0x07,
    0x03, 0x03, 0x30, 0x1D, 0x06, 0x03, 0x55, 0x1D,
    0x0E, 0x04, 0x16, 0x04, 0x14, 0xF2, 0x9B, 0x3F,
    0xD4, 0xCF, 0x15, 0x69, 0xE6, 0x23, 0xEF, 0x60,
    0xD6, 0x03, 0xAE, 0xCA, 0xC9, 0x0E, 0x54, 0x08,
    0xBB, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48,
    0xCE, 0x3D, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00,
    0x30, 0x45, 0x02, 0x21, 0x00, 0xF2, 0x3F, 0x56,
    0x45, 0x54, 0x38, 0x72, 0xCD, 0x6A, 0x43, 0xD5,
    0xDD, 0x09, 0x15, 0x35, 0xB3, 0xE3, 0x25, 0x41,
    0x8A, 0xA1, 0xE5, 0x98, 0x54, 0xCF, 0xFD, 0x39,
    0xED, 0xDB, 0x59, 0xDF, 0x5A, 0x02, 0x20, 0x7E,
    0x9C, 0xD0, 0x7F, 0x62, 0xC9, 0x62, 0xB5, 0x53,
    0x92, 0xA0, 0x3F, 0x64, 0x46, 0xC6, 0x6E, 0xDB,
    0xBD, 0xEB, 0x87, 0x8E, 0xAD, 0x16, 0x9C, 0x6F,
    0xF2, 0xC1, 0xEC, 0x55, 0x00, 0xCF, 0x10
};
const uint16_t arm_uc_default_certificate_size = sizeof(arm_uc_default_certificate);
