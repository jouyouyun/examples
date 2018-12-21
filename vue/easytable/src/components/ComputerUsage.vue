<template>
  <div class="cm_usage">
    <div class="cm_title">{{dpartName}}使用情况</div>
    <v-table is-horizontal-resize is-vertical-resize style="width:80%;" :columns="columns" :table-data="tableData" :cell-merge="cellMerge" @on-row-operation="handleOperation"></v-table>
  </div>
</template>

<script>
  import Vue from 'vue';

  // table header: 编号 名称 领用人 系统 办公软件
  export default {
    name: 'computer-usage-table',
    data() {
      return {
        dpartData: [],
        tableData: [],
        columns: [
          {field: 'index', title: '序号', width: 50, titleAlign: 'center', columnAlign: 'center', isFrozen: true},
          {field: 'id', title: '编号', width: 80, titleAlign: 'center', columnAlign: 'center', isResize: true},
          {field: 'name', title: '名称', width: 100, titleAlign: 'center', columnAlign: 'center', isResize: true},
          {field: 'owner', title: '领用人', width: 100, titleAlign: 'center', columnAlign: 'center', isResize: true},
          {field: 'os', title: '操作系统', width: 150, titleAlign: 'center', columnAlign: 'center', isResize: true},
          {field: 'office', title: '办公软件', width: 150, titleAlign: 'center', columnAlign: 'center', isResize: true},
          {field: 'operation', title: '操作', width: 150, titleAlign: 'center', columnAlign: 'center', isResize: true}
        ]
      }
    },
    computed: {
      dpartID: function() {
        return this.$route.params.id;
      },
      dpartName: function() {
        return this.getIDName(this.dpartID);
      }
    },
    methods: {
      getIDName: function(id) {
        switch (id) {
          case '1':
            return '开发部';
          case '2':
            return '测试部';
        }
        return '';
      },
      setDPartData: function(id) {
        this.$http.get('/static/data/'+id+'.json').then((response) => {
          this.dpartData = JSON.parse(response.bodyText);
        }, (error) => {
          console.log("[Error] Failed to get id data:", id, error);
        });
      },
      convertToTableData: function(datas) {
        let tmpList = [];
        for (let i in datas) {
          let officeList = datas[i]['office'];
          for (let j in officeList) {
            let tmp = {
              index: -1,
              id: '',
              name: '',
              owenr: '',
              os: '',
              office: officeList[j],
              officeLen: officeList.length
            };
            if (j == 0) {
              tmp.index = parseInt(i) + 1;
              tmp.id = datas[i]['id'];
              tmp.name = datas[i]['name'];
              tmp.owner = datas[i]['owner'];
              tmp.os = datas[i]['os'];
            }
            tmpList.push(tmp);
          }
        }
        this.tableData = tmpList;
      },
      cellMerge: function(rowIndex, rowData, field) {
        if (rowData['index'] == -1 || field == 'office') {
          return;
        }
        let ret = {
          colSpan: 1,
          rowSpan: rowData['officeLen'],
          content: rowData[field],
          componentName: ''
        }
        if (field != "operation") {
          return ret;
        }
        ret.content = '';
        ret.componentName = 'row-operation';
        return ret;
      },
      handleOperation: function(params) {
        // TODO(jouyouyun): why no signal recieved?
        console.log("[Info] [Handle Operation] click:", params);
      }
    },
    watch: {
      'dpartData': {
        handler: function(nv, ov) {
          this.convertToTableData(nv);
        }
      }
    },
    mounted: function() {
      this.setDPartData(this.dpartID);
    }
  }

  Vue.component('row-operation', {
    template: `<span>
       <a href="" @click.stop.prevent="execOperation">详情</a>
    </span>`,
    props: {
      rowData: {
        type: Object
      },
      index: {
        type: Number
      },
      field: {
        type: String
      }
    },
    methods: {
      execOperation: function() {
        let params = {rowData: this.rowData, field: this.field};
        this.$emit('on-row-operation', params);
        console.log("[Info] [Row Operation] click:", JSON.stringify(params));
      }
    }
  })
</script>

<style>
  .cm_usage {
    display: -webkit-flex;
    flex-direction: column;
    align-items: center;
    width: 100%;
  }
  .cm_title {
    margin: 80px 0 30px 0;
  }
  .cm_table {
    width: 1000px !important;
  }
</style>
