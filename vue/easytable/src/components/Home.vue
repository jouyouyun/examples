<template>
  <div class="home">
    <div class="home_title">电脑使用情况统计</div>
    <div class="home_list">
      <div class="home_item" v-for="item in itemList" @click="showItem(item)">
        {{item.name}}
      </div>
    </div>
  </div>
</template>

<script>
export default {
  data() {
    return {
      itemList: [
        {
          id: 1,
          name: "开发部",
        },
        {
          id: 2,
          name: "测试部",
        }
      ]
    };
  },
  methods: {
    showItem: function(item) {
      console.log("[showItem] item:", item.name, item.id);
      this.$router.push("/show/"+item.id);
    }
  }
}
</script>

<style>
.home {
  display: -webkit-flex;
  flex-direction: column;
  margin: 50px 0 30px 0;
  align-items: center;
}
.home_title {
  margin-top: 30px;
  font-size: 28px;
}
.home_list {
  display: -webkit-flex;
  flex-direction: column;
  font-size: 20px;
}
.home_item {
  margin-top: 15px;
}
.home_item:hover{
  cursor: pointer;
}
</style>
