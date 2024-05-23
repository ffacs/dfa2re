#include <bits/stdc++.h>

struct Node {
  Node(bool isValueNode = false) : isValueNode_(isValueNode) {}
  bool isValueNode_;
  std::vector<std::unique_ptr<Node>> children_;
  void copychildren_(const Node *other) {
    std::for_each(other->children_.begin(), other->children_.end(),
                  [&](const std::unique_ptr<Node> &child) {
                    children_.emplace_back(child->copy());
                  });
  }
  virtual std::unique_ptr<Node> copy() const = 0;
  virtual std::string toString() const = 0;
};

struct OrNode : public Node {
  std::string toString() const {
    std::stringstream ss;
    ss << "(";
    for (int i = 0; i < children_.size(); i++) {
      ss << children_[i]->toString();
      if (i + 1 != children_.size()) {
        ss << "|";
      }
    }
    ss << ")";
    return ss.str();
  }
  std::unique_ptr<Node> copy() const {
    std::unique_ptr<Node> newNode(new OrNode);
    newNode->copychildren_(this);
    return newNode;
  }
};

struct AndNode : public Node {
  std::string toString() const {
    std::stringstream ss;
    for (int i = 0; i < children_.size(); i++) {
      ss << children_[i]->toString();
    }
    return ss.str();
  }
  std::unique_ptr<Node> copy() const {
    AndNode *newNode = new AndNode;
    newNode->copychildren_(this);
    return std::unique_ptr<Node>(newNode);
  }
};

struct StarNode : public Node {
  std::string toString() const {
    std::stringstream ss;
    ss << "(";
    for (const auto &child : children_) {
      ss << child->toString();
    }
    ss << ")*";
    return ss.str();
  }
  std::unique_ptr<Node> copy() const {
    std::unique_ptr<StarNode> newNode(new StarNode);
    newNode->copychildren_(this);
    return newNode;
  }
};

struct ValueNode : public Node {
  ValueNode(bool isTerminator, const std::string &value)
      : Node(true), isTerminator_(isTerminator), value_(value) {}
  bool isTerminator_;
  std::string value_;
  std::string toString() const { return value_; }
  std::unique_ptr<Node> copy() const {
    return std::unique_ptr<ValueNode> (new ValueNode(isTerminator_, value_));
  }
};

struct Vertex {
  std::string name_;
  std::unique_ptr<OrNode> edges_;
  
  Vertex(const std::string name, std::unique_ptr<OrNode> node) :name_(name), edges_(std::move(node)) {}
  
  std::set<uint32_t> findEndWith(const std::string& name) {
    std::set<uint32_t> endWith;
    for (uint32_t i = 0; i < edges_->children_.size(); i++) {
      const auto &edge = edges_->children_[i];
      if (edge->children_.empty())
        continue;
      const Node *lastNode = edge->children_.back().get();
      if (!lastNode->isValueNode_)
        continue;
      const ValueNode *valueNode = dynamic_cast<const ValueNode *>(lastNode);
      if (!valueNode->isTerminator_)
        continue;
      if (valueNode->value_ == name) {
        endWith.insert(i);
      }      
    }
    return endWith;    
  }

  void replaceWith(std::string other, const std::unique_ptr<OrNode> &replaceMent) {
    std::set<uint32_t> endWithOther = findEndWith(other);
    auto result = std::make_unique<OrNode>();
    for (uint32_t idx = 0; idx < edges_->children_.size(); idx++) {
      if (endWithOther.find(idx) == endWithOther.end()) {
        result->children_.emplace_back(std::move(edges_->children_[idx]));
        continue;
      }
      auto item = std::move(edges_->children_[idx]);
      item->children_.pop_back();
      for (const auto& otherAnd : replaceMent->children_) {
        auto andNode = item->copy();
        for (const auto& child : otherAnd->children_) {
          andNode->children_.emplace_back(child->copy());
        }
        result->children_.emplace_back(std::move(andNode));
      }
    } 
    edges_ = std::move(result);
  }

  void selfReplace() {
    auto endWithThis = findEndWith(this->name_);
    std::set<uint32_t> notEndWith;
    auto orNode = std::make_unique<OrNode>();
    for (auto idx : endWithThis) {
      auto item = std::move(edges_->children_[idx]);
      item->children_.pop_back();
      orNode->children_.emplace_back(std::move(item));
    }
    auto starNode = std::make_unique<StarNode>();
    starNode->children_.emplace_back(std::move(orNode));

    for (uint32_t i = 0; i < edges_->children_.size(); i++) {
      if (endWithThis.find(i) == endWithThis.end()) {
        notEndWith.insert(i);
      }
    }
    
    auto result = std::make_unique<OrNode>();
    if (notEndWith.empty()) {
      auto andNode = std::make_unique<AndNode>();
      andNode->children_.emplace_back(starNode->copy());
      result->children_.emplace_back(std::move(andNode));
    } else {
      for (auto idx : notEndWith) {
        auto item = std::move(edges_->children_[idx]);
        auto andNode = std::make_unique<AndNode>();
        andNode->children_.emplace_back(starNode->copy());
        for (auto &child : item->children_) {
          andNode->children_.emplace_back(std::move(child));
        }
        result->children_.emplace_back(std::move(andNode));
      }
    }
    
    edges_ = std::move(result);
  }
};

int main() {
  std::vector<Vertex> vertexs;
  {
    std::unique_ptr<OrNode> orNode(new OrNode);
    std::unique_ptr<AndNode> andNode1(new AndNode);
    andNode1->children_.emplace_back(new ValueNode(false, "[0369]"));
    andNode1->children_.emplace_back(new ValueNode(true, "LEFT2"));
    std::unique_ptr<AndNode> andNode2(new AndNode);
    andNode2->children_.emplace_back(new ValueNode(false, "[147]"));
    andNode2->children_.emplace_back(new ValueNode(true, "LEFT0"));
    std::unique_ptr<AndNode> andNode3(new AndNode);
    andNode3->children_.emplace_back(new ValueNode(false, "[258]"));
    andNode3->children_.emplace_back(new ValueNode(true, "LEFT1"));
    orNode->children_.emplace_back(std::move(andNode1));
    orNode->children_.emplace_back(std::move(andNode2));
    orNode->children_.emplace_back(std::move(andNode3));

    Vertex left2("LEFT2", std::move(orNode));

    vertexs.push_back(std::move(left2));
  }
  {
    std::unique_ptr<OrNode> orNode(new OrNode);
    std::unique_ptr<AndNode> andNode1(new AndNode);
    andNode1->children_.emplace_back(new ValueNode(false, "[0369]"));
    andNode1->children_.emplace_back(new ValueNode(true, "LEFT1"));
    std::unique_ptr<AndNode> andNode2(new AndNode);
    andNode2->children_.emplace_back(new ValueNode(false, "[147]"));
    andNode2->children_.emplace_back(new ValueNode(true, "LEFT2"));
    std::unique_ptr<AndNode> andNode3(new AndNode);
    andNode3->children_.emplace_back(new ValueNode(false, "[258]"));
    andNode3->children_.emplace_back(new ValueNode(true, "LEFT0"));
    orNode->children_.emplace_back(std::move(andNode1));
    orNode->children_.emplace_back(std::move(andNode2));
    orNode->children_.emplace_back(std::move(andNode3));

    Vertex left1("LEFT1", std::move(orNode));

    vertexs.push_back(std::move(left1));
  }
  {
    std::unique_ptr<OrNode> orNode(new OrNode);
    std::unique_ptr<AndNode> andNode1(new AndNode);
    andNode1->children_.emplace_back(new ValueNode(false, "[0369]"));
    andNode1->children_.emplace_back(new ValueNode(true, "LEFT0"));
    std::unique_ptr<AndNode> andNode2(new AndNode);
    andNode2->children_.emplace_back(new ValueNode(false, "[147]"));
    andNode2->children_.emplace_back(new ValueNode(true, "LEFT1"));
    std::unique_ptr<AndNode> andNode3(new AndNode);
    andNode3->children_.emplace_back(new ValueNode(false, "[258]"));
    andNode3->children_.emplace_back(new ValueNode(true, "LEFT2"));
    orNode->children_.emplace_back(std::move(andNode1));
    orNode->children_.emplace_back(std::move(andNode2));
    orNode->children_.emplace_back(std::move(andNode3));

    Vertex left0("LEFT0", std::move(orNode));

    vertexs.push_back(std::move(left0));
  }
  
  for (int i = 0; i < 3; i++) {
    vertexs[i].selfReplace();
    for (int j = i + 1; j < 3; j++) {
      vertexs[j].replaceWith(vertexs[i].name_, vertexs[i].edges_);
    }
  }
  
  std::cout << vertexs[2].edges_->toString();

}