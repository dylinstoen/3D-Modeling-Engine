/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef CACHEABLE_H
#define CACHEABLE_H

class Cacheable {
public:
    Cacheable() : version_(0), source_(nullptr) { }
    Cacheable(const Cacheable* source) : source_(source) { version_ = source!=nullptr?0:1; }

    bool IsDirty() const { return source_ != nullptr && version_ != source_->version_; }
    void MarkUpdated() { if (source_ != nullptr) version_ = source_->version_; }
    void MarkDirty() { if (source_ == nullptr) version_++; }
    uint64_t GetVersion() { return version_; }
protected:
    uint64_t version_;
    const Cacheable* source_;
};

#endif // CACHEABLE_H
