/*
 * Copyright (C) 2017~2017 by CSSlayer
 * wengxt@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */

#include "libime/tablebaseddictionary.h"
#include "libime/tablecontext.h"
#include "libime/tableime.h"
#include "testutils.h"
#include <fcitx-utils/log.h>

using namespace libime;

class TestTableResolver : public TableDictionrayResolver {
public:
    TestTableResolver(boost::string_view sys, boost::string_view usr)
        : sys_(sys), usr_(usr) {}

    TableBasedDictionary *requestDict(boost::string_view name) override {
        if (name != "wbx") {
            return nullptr;
        }
        auto dict = new TableBasedDictionary;
        dict->load(sys_.c_str(), TableFormat::Binary);
        TableOptions options;
        options.setLanguageCode("zh_CN");
        dict->setTableOptions(options);
        return dict;
    }

    void saveDict(TableBasedDictionary *dict) override {
        if (usr_.empty()) {
            return;
        }
        dict->saveUser(usr_.c_str(), TableFormat::Binary);
    }

private:
    std::string sys_, usr_;
};

class TestLmResolver : public LanguageModelResolver {
public:
    TestLmResolver(boost::string_view path) : path_(path.to_string()) {}

    std::string
    languageModelFileForLanguage(boost::string_view language) override {
        if (language == "zh_CN") {
            return path_;
        }
        return {};
    }

    std::string path_;
};

int main(int argc, char *argv[]) {
    if (argc < 3) {
        return 1;
    }
    TableIME ime(std::make_unique<TestTableResolver>(argv[1], argv[2]),
                 std::make_unique<TestLmResolver>(argv[3]));
    auto dict = ime.requestDict("wbx");
    TableContext c(*dict, *ime.languageModelForDictionary(dict));
    auto printTime = [](int t) {
        std::cout << "Time: " << t / 1000000.0 << " ms" << std::endl;
    };

    std::string word;
    while (std::cin >> word) {
        bool printAll = false;
        ScopedNanoTimer t(printTime);
        if (word == "back") {
            c.backspace();
        } else if (word == "reset") {
            c.clear();
        } else if (word.size() == 1 && c.isValidInput(word[0])) {
            c.type(word);
        } else if (word == "all") {
            printAll = true;
        }
    }

    return 0;
}