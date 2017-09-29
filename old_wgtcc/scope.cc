#include "scope.h"

#include "ast.h"

#include <cassert>
#include <iostream>


Identifier* Scope::Find(const Token* tok) {
  auto ret = Find(tok->value_);
  if (ret) ret->SetTok(tok);
  return ret;
}


Identifier* Scope::FindInCurScope(const Token* tok) {
  auto ret = FindInCurScope(tok->value_);
  if (ret) ret->SetTok(tok);
  return ret;
}


Identifier* Scope::FindTag(const Token* tok) {
  auto ret = FindTag(tok->value_);
  if (ret) ret->SetTok(tok);
  return ret;
}


Identifier* Scope::FindTagInCurScope(const Token* tok) {
  auto ret = FindTagInCurScope(tok->value_);
  if (ret) ret->SetTok(tok);
  return ret;
}


void Scope::Insert(Identifier* ident) {
  Insert(ident->Name(), ident);
}


void Scope::InsertTag(Identifier* ident) {
  Insert(TagName(ident->Name()), ident);
}


Identifier* Scope::Find(const Symbol& name) {
  auto ident = identMap_.find(name);
  if (ident != identMap_.end())
    return ident->second;
  if (type_ == S_FILE || parent_ == nullptr)
    return nullptr;
  return parent_->Find(name);
}


Identifier* Scope::FindInCurScope(const Symbol&  name) {
  auto ident = identMap_.find(name);
  if (ident == identMap_.end())
    return nullptr;
  return ident->second;
}


void Scope::Insert(const Symbol&  name, Identifier* ident) {
  assert(FindInCurScope(name) == nullptr);
  identMap_[name] = ident;
}


Identifier* Scope::FindTag(const Symbol&  name) {
  auto tag = Find(TagName(name));
  if (tag) assert(tag->ToTypeName());
  return tag;
}


Identifier* Scope::FindTagInCurScope(const Symbol&  name) {
  auto tag = FindInCurScope(TagName(name));
  assert(tag == nullptr || tag->ToTypeName());
  return tag;
}


Scope::TagList Scope::AllTagsInCurScope() const {
  TagList tags;
  for (auto& kv: identMap_) {
    if (IsTagName(kv.first))
      tags.push_back(kv.second);
  }
  return tags;
}


void Scope::Print() {
  std::cout << "scope: " << this << std::endl;

  auto iter = identMap_.begin();
  for (; iter != identMap_.end(); ++iter) {
    auto name = iter->first;
    auto ident = iter->second;
    if (ident->ToTypeName()) {
      std::cout << name << "\t[type:\t"
                << ident->Type()->Str() << "]" << std::endl;
    } else {
      std::cout << name << "\t[object:\t";
      std::cout << ident->Type()->Str() << "]" << std::endl;
    }
  }
  std::cout << std::endl;
}
