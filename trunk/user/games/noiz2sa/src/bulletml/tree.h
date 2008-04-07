/// ��ʓI�ȃc���[�B�Đ��Y����

#ifndef TREE_H_
#define TREE_H_

#include "bulletmlcommon.h"

#include <list>

/// �c���[�̃N���X
/**
 * �c���[���Ă̂̓R���e�i�����݂��Ȃ��W���̂ł���Ǝv���B
 * ��ŁA�m�[�h���Ă���������тт��N���X�̏W�����R���e�i�ł���ƁB
 * �ŃC���^�[�t�F�C�X�́A
 * class YourNode : public TreeNode<YourNode>;
 * ���ċ���B
 * �|�C���^�Ǘ���O��Ƃ��Ă���B
 * �C���X�^���X�̊Ǘ��͕��i�͂��Ȃ����ǁA
 * setReleaseDuty ���Ă΂ꂽ�m�[�h���j�󂳂��ƁA
 * ����̑��q�ȉ��̐���͑S�Ĕj�󂳂��B
 */
template <class C_>
class TreeNode {
public:
    // ������e���v���[�g�����ō�����������݌v�ɂ������̂���
    typedef std::list<C_*> Children;
    typedef typename Children::iterator ChildIterator;
    typedef typename Children::const_iterator ConstChildIterator;

public:
    DECLSPEC TreeNode() {
		releaseDuty_ = false;
    }
    DECLSPEC virtual ~TreeNode();

    DECLSPEC void addChild(C_* c) {
		c->setParent(dynamic_cast<C_*>(this));
		children_.push_back(c);
    }
    DECLSPEC void setReleaseDuty(bool bl) {
		releaseDuty_ = bl;
    }
    DECLSPEC void setParent(C_* c) {
		parent_ = c;
    }

    DECLSPEC ChildIterator childBegin() { return children_.begin(); }
    DECLSPEC ChildIterator childEnd() { return children_.end(); }
	DECLSPEC size_t childSize() { return children_.size(); }
    DECLSPEC ConstChildIterator childBegin() const { return children_.begin(); }
    DECLSPEC ConstChildIterator childEnd() const { return children_.end(); }
    DECLSPEC C_* getParent() { return parent_; }

private:
    Children children_;
    C_* parent_;
    bool releaseDuty_;
};

template <class C_>
TreeNode<C_>::~TreeNode() {
    if (releaseDuty_) {
		ChildIterator ite;
		for (ite = children_.begin(); ite != children_.end(); ite++) {
			(*ite)->setReleaseDuty(true);
			delete *ite;
		}
    }
}

#endif // ! TREE_H_
