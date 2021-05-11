###################################
########### 基本配置命令 ##########
# 初始化仓库
	git init
# 下载项目
	git clone url
# 显示当前git配置
	git config --list
# 编辑git配置文件
	git config -e [--global]
# 设置提交代码时的用户信息
	git config [--global] user.name "name"
	git config [--global] user.emai "1762077903@qq.com"

###################################
########### 基本提交命令 ##########
# 添加指定文件到暂存区
	git add [file1] [file2] ...
# 添加指定目录到暂存区,包括子目录
	git add [dir]
# 添加变化前,都会要求确认,对于同一个文件的多处变化可以实现分次提交
	git add -p
# 删除工作区文件,并将这次删除放入暂存区
	git rm [file1] [file2] ...
# 改名文件,并将这个改名放入暂存区
	git mv [file_original] [file_renamed]
# 提交暂存区到仓库
	git commit -m [message]
	// message 表示此次提交说明
# 提交工作区到暂存区和仓库
	git commit -am [message]
# 修改上一次提交的说明注释
	git commit --amend -m [message]
# 从做上一次 commit ,并包括指定文件的新变化
	git commit --amend [file1] [file2] ...

###################################
########### 基本标记命令 ##########
# 列出所有 tag 
	git tag
# 新建一个 tag 在当前 commit
	git tag [tagName]
# 新建一个 tag 在指定 commit
	git tag [tagName] [commit]
# 删除本地 tag
	git tag -d [tagName]
# 删除远程 tag
	git push origin :refs/tags/[tagName]
# 查看 tag 信息
	git  show [tagName]
# 提交指定 tag 
	git push [remote] [tagName]
# 提交所有 tag 
	git push [remote] --tags
	
###################################
########### 基本节点命令 ##########
# 新建一个分支指向某个 tag
	git checkout -b [branchName] [tagName]
# 将修改的内容保存起来
	git stash
# 显示之前保存的节点
	git stash list
# 恢复之前保存的节点
	git stash apply [node_id]
# 恢复之前保存的节点,并删除保存
	git stash pop
# 删除指定保存的节点
	git stash pop [node_id]
# 清空保存的节点
	git stash clear
	
###################################
########### 基本分支命令 ##########
# 列出所有本地分支
	git branch
# 列出所有远程分支
	git branch -r
# 列出所有远程和本地分支
	git branch -a 
# 新建一个分支但停留在当前分支
	git branch [branchName]
# 新建一个分支并切换至新建分支
	git checkout -b [branchName]
# 新建一个分支并指向指定commit
	git branch [branchName] [commit]
# 新建一个分支,与远程的分支建立跟踪关系
	git branch --track [branchName] [remote_branch]
# 切换到指定分支并更新工作区
	git checkout [branchName]
# 切换到上一个分支
	git checkout -
# 建立追踪关系,在现有分支与指定分支之间
	git branch --set-upstream [branchName] [remote_branch]
# 合并指定分支到当前分支
	git merge [branchName]
# 选择一个 commit 合并到当前分支
	git cherry-pick [commit]
# 删除分支
	git branch -d [remote_branch]
# 删除远程分支
	git push origin --delete [branchName]
	git push origin :[branchName] --git branch -dr [remote/branch]

###################################
###########              ##########
# 下载远程仓库的所有变动
	git fetch [remote]
# 显示所有远程仓库
	git remote -v
# 显示某个远程仓库信息
	git remote show [remote]
# 增加一个新的远程仓库并命名
	git remote add [shorName] [url]
# 取回远程仓库的变化并与本地分支合并
	git pull [remote] [branch]
# 上传本地指定分支到远程仓库
	git push [remote] [branch]
# 强行推送当前分支到远程仓库,即使有冲突
	git push [remote] --force
# 推送所有分支到远程仓库
	git push [remote] --all

###################################
########### 变更文件命令 ##########
# 显示有变更的文件
	git status
# 显示当前分支的版本历史
	git log
	git log --pretty=oneline
# 显示当前分支最近几次提交
	git reflog
# 显示当前分支近 5 次提交
	git log -5
# 显示所有提交过得用户,提交次数排序
	git shortlog -sn
# 显示指定文件是什么人什么时间修改过
	git blame [file]
# 显示暂存区集合工作区之间的差异
	git diff
# 显示工作区与当前分支最新 commit 之间的差异 	
	git diff HEAD
# 显示两次提交之间的差异
	git diff [first-branch]...[second-branch]
# 显示今天你写了多少行代码
	git diff --shortstat "@{0 day ago}"
# 显示某次提交的元数据和内容变化
	git show [commit]
# 显示某次提交发生变化的文件
	git show --name-only
# 显示某次提交时,某个文件的内容
	git show [commit]:[filename]
	
# 恢复暂存区的指定文件到工作区
	git checkout [file]
# 恢复某个 commit 的指定文件到暂存区和工作区
	git checkout [commit] [file]
# 恢复暂存区的所有文件到工作区
	git checkout .
# 重置暂存区的指定文件,与上一次 commit 保持一致,但工作区不变
	git reset [file]
# 重置工作区与暂存区
	git reset --hard
# 重置当前分支的指针为指定commit,同时重置暂存区,但工作区不变
	git reset [commit]
# 重置当前HEAD为指定comiit,但保持暂存区和工作区不变
	git reset --keep [commit]
# 新建一个commit,用来撤销指定 commit ,后者所有变化都将被前者抵消,并且应用到当前分支
	git revert [commit]

	
	