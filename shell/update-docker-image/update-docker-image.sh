#!/bin/bash

function usage() {
    echo "Usage: $0 <image_name> <image_tag> <dockerfile>"
    exit 0
}

# update docker the special image
function update_image() {
    docker pull $1 || echo "Failed to pull image $1" || exit -1
}

# restart docker compose
function restart_docker_compose() {
    dockerfile=$1
    docker compose -f $dockerfile down
    docker compose -f $dockerfile up -d
}

# list images and remove the special image with tag
# output format: REPOSITORY  TAG  IMAGE ID  CREATED  SIZE
function clean_images() {
    deleted_image="$1:<$2>"
    dockerfile="$3"

    mapfile -t images < <(docker images --format "table {{.Repository}} {{.Tag}} {{.ID}}")
    for image in "${images[@]}"; do
        # check image whether empty line
        if [ "$image" == "" ]; then
            continue
        fi

        # parse output to items
        items=($image)
        # check items[0] whether prompt
        if [ "${items[0]}" == "REPOSITORY" ]; then
            continue
        fi
        
        # check items[0] and  items[2] whether deleted image
        if [ "${items[0]}:${items[1]}" == "${deleted_image}" ]; then
            echo "Will remove image: ${items[0]}:${items[1]}"
            restart_docker_compose "$dockerfile"
            docker rmi -f ${items[2]}
        fi
    done
}

function main() {
    # check args number
    if [ "$#" -ne "3" ]; then
        usage
    fi

    export PATH=/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin

    image_name=$1
    image_tag=$2
    dockerfile=$3

    update_image "$image_name:$image_tag"
    clean_images "$image_name" "none" "$dockerfile"
}

main $@
