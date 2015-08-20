

// Goals:
// * Optional Qt dependency?
//   * Automatically reload files that change on disk
// * Save newly-inserted values to a file.  Three options:
//   * Save to root file
//   * Save to file that makes most sense based on scope
//     * Automatically save new Robot2008 values to the Robot2008 file
//   * Save to file that makes most sense based on key path
//     * Automatically save motion control values to the file with other motion
//     values
// * Internal tree representation should be file type independent
//   * It should be fairly easy to write read/write functions for JSON, XML, and
//   others

// * should you be able to construct ConfigValues outside of a Context?

// * super easy integration with Qt.  Should have a QtAbstractItemModel, right?

// * custom UI elements?
//   * QAbstractItemView has "delegates" for this purpose

// * Custom object types?
//   * This probably can't be done with JSON :/... it would have to be XML

// * implement iterators!

// * Support many types
//   * float, double
//   * int
//   * std::string
//   *

/*
{
    "display": {
        "color" {
            "$$Robot2008": {
                "$$Robot7": {
                    "{r: 3, g: 8, b: 3}"
                }
            }
        }
    }
}

{
    "display": {
        "color" {
            "$$Robot2008": {
                "$$Robot7": {
                    {
                        r: 3,
                        g: 8,
                        b: 3
                    }
                }
            }
        }
    }
}

{
    "display": {
        "color" {
            "r": {
                "$$Robot2008": {
                    "$$Robot7": {
                        3
                    }
                }
            },
            "g": {
                "$$Robot2008": {
                    "$$Robot7": {
                        8
                    }
                }
            "b": {
                "$$Robot2008": {
                    "$$Robot7": {
                        3
                    }
                }
            }
        }
    }
}

*/

// to error: Wdelete-non-virtual-dtor

class Node {};

class ValueNode : public Node {};

class BranchNode : public Node {};

class Tree {};

Tree ReadJson(const Json::Value& json) { return Tree(); }
