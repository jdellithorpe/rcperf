/* Copyright (c) 2009-2015 Stanford University
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(S) DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>

#include <cmath>
#include <iostream>
#include <fstream>

#include "ClusterMetrics.h"
#include "Context.h"
#include "Cycles.h"
#include "Dispatch.h"
#include "ShortMacros.h"
#include "Crc32C.h"
#include "ObjectFinder.h"
#include "OptionParser.h"
#include "RamCloud.h"
#include "Tub.h"
#include "IndexLookup.h"
#include "TableEnumerator.h"
#include "Transaction.h"

using namespace RAMCloud;

/* Sweeping parameters in configuration file. Each is suffixed with {_start,
 * _end, _points, _mode} to specify the start of, end of, points in, and step
 * mode of, the parameter range for sweeping. In parentheses is the shorthand
 * code for this parameter in output file names. Unless noted otherwise for an
 * experiment, they have the listed meanings.
 *   - ds_size (dss): The size of datasets, in terms of total RAMCloud object
 *   bytes.
 *   - key_size (ks): The size of RAMCloud object keys in bytes.
 *   - multi_size (ms): The number of objects in a RAMCloud multi{read,write}.
 *   - server_size (ss): The number of RAMCloud servers to use in the 
 *   experiment.
 *   - value_size (vs): The size of RAMCloud object values in bytes.
 *
 * Fixed parameters (maintain their value during experiment, not swept).
 *   - samples_per_point (spp): Number of measurements to take for each data 
 *       point.
 *
 * Experiments:
 *   - read: Measures the latency of RAMCloud object reads over various key and
 *   value sizes.
 *     - Parameters:
 *       - key_size
 *       - value_size
 *       - samples_per_point
 *   - write: Measures the latency of RAMCloud object writes over various key
 *   and value sizes.
 *     - Parameters:
 *       - key_size
 *       - value_size
 *       - samples_per_point
 *   - multiread: Measures the latency of RAMCloud multireads over various key,
 *   value, multiread sizes, and number of servers.
 *     - Parameters:
 *       - key_size
 *       - value_size
 *       - multi_size
 *       - server_size
 *       - samples_per_point
 *   - multiread_fixeddss: Measures the latency of RAMCloud multireads over
 *   various multiread sizes, where object sizes are automatically calculated to
 *   be ds_size / multi_size, effectively holding the total number of bytes read 
 *   per multiread constant at ds_size while varying multi_size. The aim of this
 *   experiment is find out the optimal number of RAMCloud objects over which to
 *   spread ds_size bytes of data.
 *     - Parameters:
 *       - ds_size
 *       - multi_size
 *       - server_size
 *       - samples_per_point
 *   - multiread_fixeddss_chunked: Measures the latency of reading a dataset of
 *   ds_size # of objects in chunks of multi_size # of objects. The aim of this
 *   experiment is, given some # of objects of a certain size, what is the
 *   optimal multiread size to use to read all the objects.
 *     - Parameters:
 *       - ds_size: Here ds_size refers to # of objects in the dataset, not the
 *       # of bytes. Total number of bytes is (ds_size)*(key_size + value_size).
 *       - key_size
 *       - value_size
 *       - multi_size
 *       - server_size
 *       - samples_per_point
 *   - readop_async: Measures the latency of asynchronous batched ReadOps
 *   over various batch sizes, key/value sizes, and number of servers. ReadOps
 *   are constructed together and then wait() is called to execute them in a
 *   batch.
 *     - Parameters:
 *       - key_size
 *       - value_size
 *       - multi_size: Here multi_size refers to the # of ReadOps batched
 *       together before being issued by a call to wait().
 *       - server_size
 *       - samples_per_point
 */

int
main(int argc, char *argv[])
try
{
    int clientIndex;
    int numClients;
    int replicas;
    std::string configFilename;

    // Set line buffering for stdout so that printf's and log messages
    // interleave properly.
    setvbuf(stdout, NULL, _IOLBF, 1024);

    // need external context to set log levels with OptionParser
    Context context(false);

    OptionsDescription clientOptions("rcperf");
    clientOptions.add_options()

        // These first two options are currently ignored. They're here so that
        // this script can be run with cluster.py.
        ("clientIndex",
         ProgramOptions::value<int>(&clientIndex)->
            default_value(0),
         "Index of this client (first client is 0; currently ignored)")
        ("numClients",
         ProgramOptions::value<int>(&numClients)->
            default_value(1),
         "Total number of clients running (currently ignored)")

        ("replicas",
         ProgramOptions::value<int>(&replicas),
         "Number of replicas configured for given RAMCloud cluster.")
        ("config",
         ProgramOptions::value<std::string>(&configFilename),
         "Configuration file for experiment specification.");
    
    OptionParser optionParser(clientOptions, argc, argv);
    context.transportManager->setSessionTimeout(
            optionParser.options.getSessionTimeout());

    LOG(NOTICE, "Connecting to %s",
        optionParser.options.getCoordinatorLocator().c_str());

    string locator = optionParser.options.getExternalStorageLocator();
    if (locator.size() == 0) {
        locator = optionParser.options.getCoordinatorLocator();
    }

    RamCloud client(&optionParser.options);

    // Default values for experiment parameters
    uint32_t key_size_start = 30;
    uint32_t key_size_end = 30;
    uint32_t key_size_points = 1;
    std::string key_size_mode = "linear";
    uint32_t value_size_start = 100;
    uint32_t value_size_end = 100;
    uint32_t value_size_points = 1;
    std::string value_size_mode = "linear";
    uint32_t ds_size_start = 100;
    uint32_t ds_size_end = 100;
    uint32_t ds_size_points = 1;
    std::string ds_size_mode = "linear";
    uint32_t multi_size_start = 32;
    uint32_t multi_size_end = 32;
    uint32_t multi_size_points = 1;
    std::string multi_size_mode = "linear";
    uint32_t server_size_start = 1;
    uint32_t server_size_end = 1;
    uint32_t server_size_points = 1;
    std::string server_size_mode = "linear";
    uint32_t samples_per_point = 1000;

    std::ifstream cfgFile(configFilename);
    std::string line;
    std::string op;
    while (true) {
      if (cfgFile.eof()) {
        printf("End of experiments\n");
        return 0;
      }

      bool foundCfg = false;
      while (std::getline(cfgFile, line)) {
        if (line[0] == '[') {
          op = line.substr(1, line.find_last_of(']') - 1);
          foundCfg = true;
        } else if (line[0] == '#') {
          // skip
          continue;
        } else if (line.size() == 0) {
          if (foundCfg == false)
            continue;
          else
            break;
        } else {
          std::string var_name = line.substr(0, line.find_first_of(' '));
          
          if (var_name.compare("key_size_start") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            key_size_start = var_int_value;
          } else if (var_name.compare("key_size_end") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            key_size_end = var_int_value;
          } else if (var_name.compare("key_size_points") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            key_size_points = var_int_value;
          } else if (var_name.compare("key_size_mode") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            key_size_mode = var_value;
          } else if (var_name.compare("value_size_start") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            value_size_start = var_int_value;
          } else if (var_name.compare("value_size_end") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            value_size_end = var_int_value;
          } else if (var_name.compare("value_size_points") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            value_size_points = var_int_value;
          } else if (var_name.compare("value_size_mode") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            value_size_mode = var_value;
          } else if (var_name.compare("ds_size_start") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            ds_size_start = var_int_value;
          } else if (var_name.compare("ds_size_end") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            ds_size_end = var_int_value;
          } else if (var_name.compare("ds_size_points") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            ds_size_points = var_int_value;
          } else if (var_name.compare("ds_size_mode") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            ds_size_mode = var_value;
          } else if (var_name.compare("multi_size_start") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            multi_size_start = var_int_value;
          } else if (var_name.compare("multi_size_end") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            multi_size_end = var_int_value;
          } else if (var_name.compare("multi_size_points") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            multi_size_points = var_int_value;
          } else if (var_name.compare("multi_size_mode") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            multi_size_mode = var_value;
          } else if (var_name.compare("server_size_start") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            server_size_start = var_int_value;
          } else if (var_name.compare("server_size_end") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            server_size_end = var_int_value;
          } else if (var_name.compare("server_size_points") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            server_size_points = var_int_value;
          } else if (var_name.compare("server_size_mode") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            server_size_mode = var_value;
          } else if (var_name.compare("samples_per_point") == 0) {
            std::string var_value = line.substr(line.find_last_of(' ') + 1, line.length());
            uint32_t var_int_value = std::stoi(var_value);
            samples_per_point = var_int_value;
          } else {
            printf("ERROR: Unknown parameter: %s\n", var_name.c_str());
            return 1;
          }
        }
      }

      if (foundCfg == false) {
        printf("End of experiments\n");
        return 0;
      }

      std::vector<uint32_t> key_sizes; 
      std::vector<uint32_t> value_sizes; 
      std::vector<uint32_t> ds_sizes; 
      std::vector<uint32_t> multi_sizes;
      std::vector<uint32_t> server_sizes;

      if (key_size_points > 1) {
        if (key_size_mode.compare("linear") == 0) {
          uint32_t step_size = 
            (key_size_end - key_size_start) / (key_size_points - 1);

          for (int i = key_size_start; i <= key_size_end; i += step_size) 
            key_sizes.push_back(i);
        } else if (key_size_mode.compare("geometric") == 0) {
          double c = pow(10, log10((double)key_size_end/(double)key_size_start) / (double)(key_size_points - 1));
          for (int i = key_size_start; i <= key_size_end; i = ceil(c * i))
            key_sizes.push_back(i);
        } else {
          printf("ERROR: Unknown points mode: %s\n", key_size_mode.c_str());
          return 1;
        }
      } else {
        key_sizes.push_back(key_size_start);
      }

      if (value_size_points > 1) {
        if (value_size_mode.compare("linear") == 0) {
          uint32_t step_size = 
            (value_size_end - value_size_start) / (value_size_points - 1);

          for (int i = value_size_start; i <= value_size_end; i += step_size) 
            value_sizes.push_back(i);
        } else if (value_size_mode.compare("geometric") == 0) {
          double c = pow(10, log10((double)value_size_end/(double)value_size_start) / (double)(value_size_points - 1));
          for (int i = value_size_start; i <= value_size_end; i = ceil(c * i))
            value_sizes.push_back(i);
        } else {
          printf("ERROR: Unknown points mode: %s\n", value_size_mode.c_str());
          return 1;
        }
      } else {
        value_sizes.push_back(value_size_start);
      }

      if (ds_size_points > 1) {
        if (ds_size_mode.compare("linear") == 0) {
          uint32_t step_size = 
            (ds_size_end - ds_size_start) / (ds_size_points - 1);

          for (int i = ds_size_start; i <= ds_size_end; i += step_size) 
            ds_sizes.push_back(i);
        } else if (ds_size_mode.compare("geometric") == 0) {
          double c = pow(10, log10((double)ds_size_end/(double)ds_size_start) / (double)(ds_size_points - 1));
          for (int i = ds_size_start; i <= ds_size_end; i = ceil(c * i))
            ds_sizes.push_back(i);
        } else {
          printf("ERROR: Unknown points mode: %s\n", ds_size_mode.c_str());
          return 1;
        }
      } else {
        ds_sizes.push_back(ds_size_start);
      }

      if (multi_size_points > 1) {
        if (multi_size_mode.compare("linear") == 0) {
          uint32_t step_size = 
            (multi_size_end - multi_size_start) / (multi_size_points - 1);

          for (int i = multi_size_start; i <= multi_size_end; i += step_size) 
            multi_sizes.push_back(i);
        } else if (multi_size_mode.compare("geometric") == 0) {
          double c = pow(10, log10((double)multi_size_end/(double)multi_size_start) / (double)(multi_size_points - 1));
          for (int i = multi_size_start; i <= multi_size_end; i = ceil(c * i))
            multi_sizes.push_back(i);
        } else {
          printf("ERROR: Unknown points mode: %s\n", multi_size_mode.c_str());
          return 1;
        }
      } else {
        multi_sizes.push_back(multi_size_start);
      }

      if (server_size_points > 1) {
        if (server_size_mode.compare("linear") == 0) {
          uint32_t step_size = 
            (server_size_end - server_size_start) / (server_size_points - 1);

          for (int i = server_size_start; i <= server_size_end; i += step_size) 
            server_sizes.push_back(i);
        } else if (server_size_mode.compare("geometric") == 0) {
          double c = pow(10, log10((double)server_size_end/(double)server_size_start) / (double)(server_size_points - 1));
          for (int i = server_size_start; i <= server_size_end; i = ceil(c * i))
            server_sizes.push_back(i);
        } else {
          printf("ERROR: Unknown points mode: %s\n", server_size_mode.c_str());
          return 1;
        }
      } else {
        server_sizes.push_back(server_size_start);
      }

      if (op.compare("read") == 0) {
        uint64_t tableId = client.createTable("test");

        for (int ks_idx = 0; ks_idx < key_sizes.size(); ks_idx++) {
          uint32_t key_size = key_sizes[ks_idx];

          // Open data file for writing.
          FILE * datFile;
          char filename[128];
          sprintf(filename, "read.spp_%d.ks_%d.csv", samples_per_point, key_size);
          datFile = fopen(filename, "w");
          fprintf(datFile, "%12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s\n", 
              "ValueSize",
              "1th",
              "2th",
              "5th",
              "10th",
              "25th",
              "50th",
              "75th",
              "90th",
              "95th",
              "98th",
              "99th");

          for (int vs_idx = 0; vs_idx < value_sizes.size(); vs_idx++) {
            uint32_t value_size = value_sizes[vs_idx];
            printf("Read Test: key_size: %dB, value_size: %dB\n", key_size, value_size);

            char randomKey[key_size];
            char randomValue[value_size];

            client.write(tableId, randomKey, key_size, randomValue, value_size);

            Buffer value;
            uint64_t latency[samples_per_point];
            for (int i = 0; i < samples_per_point; i++) {
              bool exists;
              uint64_t start = Cycles::rdtsc();
              client.read(tableId, randomKey, key_size, &value, NULL, NULL, &exists);
              uint64_t end = Cycles::rdtsc();
              latency[i] = Cycles::toNanoseconds(end-start);
            }

            std::vector<uint64_t> latencyVec(latency, latency+samples_per_point);

            std::sort(latencyVec.begin(), latencyVec.end());

            uint64_t sum = 0;
            for (int i = 0; i < samples_per_point; i++) {
              sum += latencyVec[i];
            }

            fprintf(datFile, "%12d %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f\n", 
                value_size,
                latencyVec[samples_per_point*1/100]/1000.0,
                latencyVec[samples_per_point*2/100]/1000.0,
                latencyVec[samples_per_point*5/100]/1000.0,
                latencyVec[samples_per_point*10/100]/1000.0,
                latencyVec[samples_per_point*25/100]/1000.0,
                latencyVec[samples_per_point*50/100]/1000.0,
                latencyVec[samples_per_point*75/100]/1000.0,
                latencyVec[samples_per_point*90/100]/1000.0,
                latencyVec[samples_per_point*95/100]/1000.0,
                latencyVec[samples_per_point*98/100]/1000.0,
                latencyVec[samples_per_point*99/100]/1000.0);
            fflush(datFile);
          }

          fclose(datFile);
        }

        client.dropTable("test");
      } else if (op.compare("write") == 0) {
        uint64_t tableId = client.createTable("test");

        for (int ks_idx = 0; ks_idx < key_sizes.size(); ks_idx++) {
          uint32_t key_size = key_sizes[ks_idx];

          // Open data file for writing.
          FILE * datFile;
          char filename[128];
          sprintf(filename, "write.spp_%d.rf_%d.ks_%d.csv", samples_per_point, replicas, key_size);
          datFile = fopen(filename, "w");
          fprintf(datFile, "%12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s\n", 
              "ValueSize",
              "1th",
              "2th",
              "5th",
              "10th",
              "25th",
              "50th",
              "75th",
              "90th",
              "95th",
              "98th",
              "99th");

          for (int vs_idx = 0; vs_idx < value_sizes.size(); vs_idx++) {
            uint32_t value_size = value_sizes[vs_idx];
            printf("Write Test: key_size: %dB, value_size: %dB\n", key_size, value_size);

            char randomKey[key_size];
            char randomValue[value_size];

            client.write(tableId, randomKey, key_size, randomValue, value_size);

            uint64_t latency[samples_per_point];
            for (int i = 0; i < samples_per_point; i++) {
              bool exists;
              uint64_t start = Cycles::rdtsc();
              client.write(tableId, randomKey, key_size, randomValue, value_size);
              uint64_t end = Cycles::rdtsc();
              latency[i] = Cycles::toNanoseconds(end-start);
            }

            std::vector<uint64_t> latencyVec(latency, latency+samples_per_point);

            std::sort(latencyVec.begin(), latencyVec.end());

            uint64_t sum = 0;
            for (int i = 0; i < samples_per_point; i++) {
              sum += latencyVec[i];
            }

            fprintf(datFile, "%12d %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f\n", 
                value_size,
                latencyVec[samples_per_point*1/100]/1000.0,
                latencyVec[samples_per_point*2/100]/1000.0,
                latencyVec[samples_per_point*5/100]/1000.0,
                latencyVec[samples_per_point*10/100]/1000.0,
                latencyVec[samples_per_point*25/100]/1000.0,
                latencyVec[samples_per_point*50/100]/1000.0,
                latencyVec[samples_per_point*75/100]/1000.0,
                latencyVec[samples_per_point*90/100]/1000.0,
                latencyVec[samples_per_point*95/100]/1000.0,
                latencyVec[samples_per_point*98/100]/1000.0,
                latencyVec[samples_per_point*99/100]/1000.0);
            fflush(datFile);
          }

          fclose(datFile);
        }

        client.dropTable("test");
      } else if (op.compare("readop_async") == 0) {
        for (int sv_idx = 0; sv_idx < server_sizes.size(); sv_idx++) {
          uint32_t server_size = server_sizes[sv_idx];

          uint64_t tableId = client.createTable("test", server_size);

          for (int ks_idx = 0; ks_idx < key_sizes.size(); ks_idx++) {
            uint32_t key_size = key_sizes[ks_idx];

            for (int vs_idx = 0; vs_idx < value_sizes.size(); vs_idx++) {
              uint32_t value_size = value_sizes[vs_idx];

              // Open data file for writing.
              FILE * datFile;
              char filename[128];
              sprintf(filename, "readop_async.spp_%d.sv_%d.ks_%d.vs_%d.csv", samples_per_point, server_size, key_size, value_size);
              datFile = fopen(filename, "w");
              fprintf(datFile, "%12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s\n", 
                  "MultiSize",
                  "1th",
                  "2th",
                  "5th",
                  "10th",
                  "25th",
                  "50th",
                  "75th",
                  "90th",
                  "95th",
                  "98th",
                  "99th");

              for (int ms_idx = 0; ms_idx < multi_sizes.size(); ms_idx++) {
                uint32_t multi_size = multi_sizes[ms_idx];
                printf("Tx Async Multiread Test: server_size: %d, multi_size: %d, key_size: %dB, value_size: %dB\n", server_size, multi_size, key_size, value_size);

                char key[key_size];
                memset(key, 0, key_size);

                // Write value_size data into objects.
                for (int i = 0; i < multi_size; i++) {
                  memcpy(key, (char*)&i, sizeof(int));
                  char randomValue[value_size];
                  client.write(tableId, key, key_size, randomValue, value_size);
                }

                int READOP_POOL_SIZE = 100;
                Tub<Transaction::ReadOp> readOps[READOP_POOL_SIZE];
                Buffer values[READOP_POOL_SIZE];

                uint64_t latency[samples_per_point];
                for (int i = 0; i < samples_per_point; i++) {
                  Transaction tx(&client);

                  uint64_t start = Cycles::rdtsc();
                  for (int j = 0; j < multi_size; j++) {
                    memcpy(key, (char*)&j, sizeof(int));
                    readOps[j % READOP_POOL_SIZE].construct(&tx, tableId, (const char*)key, key_size, &values[j % READOP_POOL_SIZE], true);

                    if ((j + 1) % READOP_POOL_SIZE == 0) {
                      for (int k = 0; k < READOP_POOL_SIZE; k++) {
                        readOps[k]->wait();
                      }
                    }
                  }

                  for (int j = 0; j < (multi_size % READOP_POOL_SIZE); j++) {
                    readOps[j]->wait();
                  }
                  uint64_t end = Cycles::rdtsc();

                  latency[i] = Cycles::toNanoseconds(end-start);
                }

                std::vector<uint64_t> latencyVec(latency, latency+samples_per_point);

                std::sort(latencyVec.begin(), latencyVec.end());

                uint64_t sum = 0;
                for (int i = 0; i < samples_per_point; i++) {
                  sum += latencyVec[i];
                }

                fprintf(datFile, "%12d %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f\n",
                    multi_size,
                    latencyVec[samples_per_point*1/100]/1000.0,
                    latencyVec[samples_per_point*2/100]/1000.0,
                    latencyVec[samples_per_point*5/100]/1000.0,
                    latencyVec[samples_per_point*10/100]/1000.0,
                    latencyVec[samples_per_point*25/100]/1000.0,
                    latencyVec[samples_per_point*50/100]/1000.0,
                    latencyVec[samples_per_point*75/100]/1000.0,
                    latencyVec[samples_per_point*90/100]/1000.0,
                    latencyVec[samples_per_point*95/100]/1000.0,
                    latencyVec[samples_per_point*98/100]/1000.0,
                    latencyVec[samples_per_point*99/100]/1000.0);
                fflush(datFile);
              } // ms_idx

              fclose(datFile);
            } // vs_idx
          } // ks_idx

          client.dropTable("test");
        } // sv_idx
      } else if (op.compare("multiread_fixeddss_chunked") == 0) {
        // Open data file for writing.
        FILE * datFile;
        char filename[128];
        sprintf(filename, "multiread_fixeddss_chunked.csv");
        datFile = fopen(filename, "w");
        fprintf(datFile, "%12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s\n", 
            "Samples",
            "DatasetSize",
            "ServerSize",
            "MultiSize",
            "KeySize",
            "ValueSize",
            "1th",
            "2th",
            "5th",
            "10th",
            "25th",
            "50th",
            "75th",
            "90th",
            "95th",
            "98th",
            "99th");

        for (int sv_idx = 0; sv_idx < server_sizes.size(); sv_idx++) {
          uint32_t server_size = server_sizes[sv_idx];

          uint64_t tableId = client.createTable("test", server_size);

          // Calculate hash ranges.
          uint64_t endKeyHashes[server_size];
          uint64_t tabletRange = 1 + ~0UL / server_size;
          for (uint32_t i = 0; i < server_size; i++) {
            uint64_t startKeyHash = i * tabletRange;
            uint64_t endKeyHash = startKeyHash + tabletRange - 1;
            if (i == (server_size - 1))
              endKeyHash = ~0UL;

            endKeyHashes[i] = endKeyHash;
          }

          for (int dss_idx = 0; dss_idx < ds_sizes.size(); dss_idx++) {
            uint32_t ds_size = ds_sizes[dss_idx];

            for (int ks_idx = 0; ks_idx < key_sizes.size(); ks_idx++) {
              uint32_t key_size = key_sizes[ks_idx];

              // Construct keys.
              char keys[ds_size][key_size];
              memset(keys, 0, ds_size * key_size);
              uint32_t key_candidate = 0;
              uint32_t n = 0;
              while (n < ds_size) {
                sprintf(keys[n], "%d", key_candidate);

                // Find the tablet this key belongs to.
                uint64_t keyHash = Key::getHash(tableId, (const void*)keys[n],
                    (uint16_t)key_size);
                uint64_t tablet = 0;
                for (uint32_t j = 0; j < server_size; j++) {
                  if (keyHash <= endKeyHashes[j]) {
                    tablet = j;
                    break;
                  }
                }
                
                if (tablet == n % server_size) {
                  n++;
                }

                key_candidate++;
              }

              for (int vs_idx = 0; vs_idx < value_sizes.size(); vs_idx++) {
                uint32_t value_size = value_sizes[vs_idx];

                // Write out dataset.
                for (int i = 0; i < ds_size; i++) {
                  char randomValue[value_size];
                  client.write(tableId, keys[i], key_size, randomValue, value_size);
                }

                for (int ms_idx = 0; ms_idx < multi_sizes.size(); ms_idx++) {
                  uint32_t multi_size = multi_sizes[ms_idx];

                  printf("Multiread Fixed DSS Chunked Test: server_size: %d, ds_size: %d, key_size: %dB, value_size: %dB, multi_size: %d\n", server_size, ds_size, key_size, value_size, multi_size);

                  // Prepare multiread data structures.
                  MultiReadObject requestObjects[multi_size];
                  MultiReadObject* requests[multi_size];
                  Tub<ObjectBuffer> values[multi_size];

                  uint64_t latency[samples_per_point];
                  for (int i = 0; i < samples_per_point; i++) {
                    uint64_t start = Cycles::rdtsc();
                    uint32_t mark = 0;
                    while (mark < ds_size) {
                      uint32_t batch_size = std::min(multi_size, ds_size - mark);
                      for (int j = 0; j < batch_size; j++) {
                        requestObjects[j] = MultiReadObject(tableId, 
                            keys[mark + j], key_size, &values[j]);
                        requests[j] = &requestObjects[j];
                      }

                      client.multiRead(requests, batch_size);

                      mark += batch_size;
                    }
                    uint64_t end = Cycles::rdtsc();
                    latency[i] = Cycles::toNanoseconds(end-start);
                  }

                  std::vector<uint64_t> latencyVec(latency, latency+samples_per_point);

                  std::sort(latencyVec.begin(), latencyVec.end());

                  uint64_t sum = 0;
                  for (int i = 0; i < samples_per_point; i++) {
                    sum += latencyVec[i];
                  }

                  fprintf(datFile, "%12d %12d %12d %12d %12d %12d %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f\n", 
                      samples_per_point,
                      ds_size,
                      server_size,
                      multi_size,
                      key_size,
                      value_size,
                      latencyVec[samples_per_point*1/100]/1000.0,
                      latencyVec[samples_per_point*2/100]/1000.0,
                      latencyVec[samples_per_point*5/100]/1000.0,
                      latencyVec[samples_per_point*10/100]/1000.0,
                      latencyVec[samples_per_point*25/100]/1000.0,
                      latencyVec[samples_per_point*50/100]/1000.0,
                      latencyVec[samples_per_point*75/100]/1000.0,
                      latencyVec[samples_per_point*90/100]/1000.0,
                      latencyVec[samples_per_point*95/100]/1000.0,
                      latencyVec[samples_per_point*98/100]/1000.0,
                      latencyVec[samples_per_point*99/100]/1000.0);
                  fflush(datFile);
                } // ms_idx
              } // vs_idx
            } // ks_idx
          } // dss_idx

          client.dropTable("test");
        } // sv_idx

        fclose(datFile);
      } else if (op.compare("multiread") == 0) {
        // Open data file for writing.
        FILE * datFile;
        char filename[128];
        sprintf(filename, "multiread.csv");
        datFile = fopen(filename, "w");
        fprintf(datFile, "%12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s\n", 
            "Samples",
            "ServerSize",
            "MultiSize",
            "KeySize",
            "ValueSize",
            "1th",
            "2th",
            "5th",
            "10th",
            "25th",
            "50th",
            "75th",
            "90th",
            "95th",
            "98th",
            "99th");

        for (int sv_idx = 0; sv_idx < server_sizes.size(); sv_idx++) {
          uint32_t server_size = server_sizes[sv_idx];

          uint64_t tableId = client.createTable("test", server_size);

          // Calculate hash ranges.
          uint64_t endKeyHashes[server_size];
          uint64_t tabletRange = 1 + ~0UL / server_size;
          for (uint32_t i = 0; i < server_size; i++) {
            uint64_t startKeyHash = i * tabletRange;
            uint64_t endKeyHash = startKeyHash + tabletRange - 1;
            if (i == (server_size - 1))
              endKeyHash = ~0UL;

            endKeyHashes[i] = endKeyHash;
          }

          for (int ms_idx = 0; ms_idx < multi_sizes.size(); ms_idx++) {
            uint32_t multi_size = multi_sizes[ms_idx];

            for (int ks_idx = 0; ks_idx < key_sizes.size(); ks_idx++) {
              uint32_t key_size = key_sizes[ks_idx];

              // Construct keys.
              char keys[multi_size][key_size];
              memset(keys, 0, multi_size * key_size);
              uint32_t key_candidate = 0;
              uint32_t n = 0;
              while (n < multi_size) {
                sprintf(keys[n], "%d", key_candidate);

                // Find the tablet this key belongs to.
                uint64_t keyHash = Key::getHash(tableId, (const void*)keys[n],
                    (uint16_t)key_size);
                uint64_t tablet = 0;
                for (uint32_t j = 0; j < server_size; j++) {
                  if (keyHash <= endKeyHashes[j]) {
                    tablet = j;
                    break;
                  }
                }
                
                if (tablet == n % server_size) {
                  n++;
                }

                key_candidate++;
              }

              for (int vs_idx = 0; vs_idx < value_sizes.size(); vs_idx++) {
                uint32_t value_size = value_sizes[vs_idx];
                printf("Multiread Test: server_size: %d, multi_size: %d, key_size: %dB, value_size: %dB\n", server_size, multi_size, key_size, value_size);

                // Write value_size data into objects.
                for (int i = 0; i < multi_size; i++) {
                  char randomValue[value_size];
                  client.write(tableId, keys[i], key_size, randomValue, value_size);
                }

                // Prepare multiread data structures.
                MultiReadObject requestObjects[multi_size];
                MultiReadObject* requests[multi_size];
                Tub<ObjectBuffer> values[multi_size];

                for (int i = 0; i < multi_size; i++) {
                  requestObjects[i] = MultiReadObject(tableId, keys[i], 
                      key_size, &values[i]);
                  requests[i] = &requestObjects[i];
                }

                uint64_t latency[samples_per_point];
                for (int i = 0; i < samples_per_point; i++) {
                  uint64_t start = Cycles::rdtsc();
                  client.multiRead(requests, multi_size);
                  uint64_t end = Cycles::rdtsc();
                  latency[i] = Cycles::toNanoseconds(end-start);
                }

                std::vector<uint64_t> latencyVec(latency, latency+samples_per_point);

                std::sort(latencyVec.begin(), latencyVec.end());

                uint64_t sum = 0;
                for (int i = 0; i < samples_per_point; i++) {
                  sum += latencyVec[i];
                }

                fprintf(datFile, "%12d %12d %12d %12d %12d %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f %12.3f\n", 
                    samples_per_point,
                    server_size,
                    multi_size,
                    key_size,
                    value_size,
                    latencyVec[samples_per_point*1/100]/1000.0/multi_size,
                    latencyVec[samples_per_point*2/100]/1000.0/multi_size,
                    latencyVec[samples_per_point*5/100]/1000.0/multi_size,
                    latencyVec[samples_per_point*10/100]/1000.0/multi_size,
                    latencyVec[samples_per_point*25/100]/1000.0/multi_size,
                    latencyVec[samples_per_point*50/100]/1000.0/multi_size,
                    latencyVec[samples_per_point*75/100]/1000.0/multi_size,
                    latencyVec[samples_per_point*90/100]/1000.0/multi_size,
                    latencyVec[samples_per_point*95/100]/1000.0/multi_size,
                    latencyVec[samples_per_point*98/100]/1000.0/multi_size,
                    latencyVec[samples_per_point*99/100]/1000.0/multi_size);
                fflush(datFile);
              } // vs_idx
            } // ks_idx
          } // ms_idx

          client.dropTable("test");
        } // sv_idx

        fclose(datFile);
      } else if (op.compare("multiread_fixeddss") == 0) {
        for (int sv_idx = 0; sv_idx < server_sizes.size(); sv_idx++) {
          uint32_t server_size = server_sizes[sv_idx];

          uint64_t tableId = client.createTable("test", server_size);

          // Calculate hash ranges.
          uint64_t endKeyHashes[server_size];
          uint64_t tabletRange = 1 + ~0UL / server_size;
          for (uint32_t i = 0; i < server_size; i++) {
            uint64_t startKeyHash = i * tabletRange;
            uint64_t endKeyHash = startKeyHash + tabletRange - 1;
            if (i == (server_size - 1))
              endKeyHash = ~0UL;

            endKeyHashes[i] = endKeyHash;
          }

          for (int dss_idx = 0; dss_idx < ds_sizes.size(); dss_idx++) {
            uint32_t ds_size = ds_sizes[dss_idx];

            // Open data file for writing.
            FILE * datFile;
            char filename[128];
            sprintf(filename, "multiread_fixeddss.spp_%d.sv_%d.dss_%d.csv", samples_per_point, server_size, ds_size);
            datFile = fopen(filename, "w");
            fprintf(datFile, "%12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s %12s\n", 
                "MultiSize",
                "1th",
                "2th",
                "5th",
                "10th",
                "25th",
                "50th",
                "75th",
                "90th",
                "95th",
                "98th",
                "99th");

            for (int ms_idx = 0; ms_idx < multi_sizes.size(); ms_idx++) {
              uint32_t multi_size = multi_sizes[ms_idx];

              printf("Multiread Fixed DSS Test: server_size: %d, ds_size: %d, multi_size: %d\n", server_size, ds_size, multi_size);

              // Compute key_size and value_size.
              uint32_t key_size = 30; // Use fixed 30B keys.
              uint32_t value_size = (ds_size - key_size) / multi_size;

              // Construct keys.
              char keys[multi_size][key_size];
              memset(keys, 0, multi_size * key_size);
              uint32_t key_candidate = 0;
              uint32_t n = 0;
              while (n < multi_size) {
                sprintf(keys[n], "%d", key_candidate);

                // Find the tablet this key belongs to.
                uint64_t keyHash = Key::getHash(tableId, (const void*)keys[n],
                    (uint16_t)key_size);
                uint64_t tablet = 0;
                for (uint32_t j = 0; j < server_size; j++) {
                  if (keyHash <= endKeyHashes[j]) {
                    tablet = j;
                    break;
                  }
                }

                if (tablet == n % server_size) {
                  n++;
                }

                key_candidate++;
              }

              // Write value_size data into objects.
              for (int i = 0; i < multi_size; i++) {
                char randomValue[value_size];
                client.write(tableId, keys[i], key_size, randomValue, value_size);
              }

              // Prepare multiread data structures.
              MultiReadObject requestObjects[multi_size];
              MultiReadObject* requests[multi_size];
              Tub<ObjectBuffer> values[multi_size];

              for (int i = 0; i < multi_size; i++) {
                requestObjects[i] = MultiReadObject(tableId, keys[i], 
                    key_size, &values[i]);
                requests[i] = &requestObjects[i];
              }

              uint64_t latency[samples_per_point];
              for (int i = 0; i < samples_per_point; i++) {
                uint64_t start = Cycles::rdtsc();
                client.multiRead(requests, multi_size);
                uint64_t end = Cycles::rdtsc();
                latency[i] = Cycles::toNanoseconds(end-start);
              }

              std::vector<uint64_t> latencyVec(latency, latency+samples_per_point);

              std::sort(latencyVec.begin(), latencyVec.end());

              uint64_t sum = 0;
              for (int i = 0; i < samples_per_point; i++) {
                sum += latencyVec[i];
              }

              fprintf(datFile, "%12d %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f %12.1f\n", 
                  multi_size,
                  latencyVec[samples_per_point*1/100]/1000.0,
                  latencyVec[samples_per_point*2/100]/1000.0,
                  latencyVec[samples_per_point*5/100]/1000.0,
                  latencyVec[samples_per_point*10/100]/1000.0,
                  latencyVec[samples_per_point*25/100]/1000.0,
                  latencyVec[samples_per_point*50/100]/1000.0,
                  latencyVec[samples_per_point*75/100]/1000.0,
                  latencyVec[samples_per_point*90/100]/1000.0,
                  latencyVec[samples_per_point*95/100]/1000.0,
                  latencyVec[samples_per_point*98/100]/1000.0,
                  latencyVec[samples_per_point*99/100]/1000.0);
              fflush(datFile);
            } // ms_idx

            fclose(datFile);
          } // dss_idx

          client.dropTable("test");
        } // sv_idx
      } else {
        printf("ERROR: Unknown operation: %s\n", op.c_str());
        return 1;
      }
    }
  
    return 0;
} catch (RAMCloud::ClientException& e) {
    fprintf(stderr, "RAMCloud exception: %s\n", e.str().c_str());
    return 1;
} catch (RAMCloud::Exception& e) {
    fprintf(stderr, "RAMCloud exception: %s\n", e.str().c_str());
    return 1;
}
